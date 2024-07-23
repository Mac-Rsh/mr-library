/**
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2024-06-20    MacRsh       First version
 */

#include <include/mr_library.h>

static mr_class_t __root_class;

static int mr_root_class_init(void)
{
    /* Initialize the root class */
    mr_class_init(&__root_class, "/", 0, NULL, NULL);
    return MR_EOK;
}
MR_INIT_SYSTEM_EXPORT("class", mr_root_class_init);

static mr_class_t *__class_find(mr_class_t *parent, const char *name)
{
    mr_list_t *list_pos;
    mr_class_t *child;

    /* Dealing with special '..' And '.' */
    if (strcmp(name, "..") == 0)
    {
        return parent->parent;
    }
    if (strcmp(name, ".") == 0)
    {
        return parent;
    }

    /* Search the class */
    MR_LIST_FOR_EACH(list_pos, &parent->clist)
    {
        /* Get the children from the list */
        child = MR_LIST_ENTRY(list_pos, mr_class_t, list);
        if (strcmp(child->name, name) == 0)
        {
            return child;
        }
    }
    return NULL;
}

/**
 * @brief This function initialize the class.
 *
 * @param class The class to initialize.
 * @param name The name of the class.
 * @param privsize The size of the private data.
 * @param release The release function(released when ref-count is 0).
 * @param methods The methods of the class.
 *
 * @note priv_size must be 0 if methods is NULL.
 */
void mr_class_init(mr_class_t *class, const char *name, size_t privsize,
                   void (*release)(mr_class_t *), void *methods)
{
    MR_ASSERT(class != NULL);
    MR_ASSERT(name != NULL);
    MR_ASSERT((privsize == 0) || (methods != NULL));

    /* Initialize the class */
    class->name = name;
    class->parent = NULL;
    mr_list_init(&class->list);
    mr_list_init(&class->clist);
    mr_spin_lock_init(&class->lock);
    class->privdata = NULL;
    class->privsize = privsize;
    mr_ref_init(&class->refcount);
    class->release = release;
    class->methods = methods;
}

static void __class_release(mr_class_t *class)
{
    /* Release the class */
    mr_free(class);
}

/**
 * @brief This function create the class.
 *
 * @param name The name of the class.
 * @param privsize The size of the private data.
 * @param methods The methods of the class.
 *
 * @return The class, or NULL if failed.
 *
 * @note priv_size must be 0 if methods is NULL.
 */
mr_class_t *mr_class_create(const char *name, uint32_t privsize, void *methods)
{
    mr_class_t *class;

    MR_ASSERT(name != NULL);
    MR_ASSERT((privsize == 0) || (methods != NULL));

    /* Create the class */
    class = mr_malloc(sizeof(mr_class_t));
    if (class == NULL)
    {
        return NULL;
    }

    /* Initialize the class */
    mr_class_init(class, name, privsize, __class_release, methods);
    return class;
}

/**
 * @brief This function add the class to the parent class.
 *
 * @param class The class to add.
 * @param parent The parent class.
 *
 * @return The error code.
 */
int mr_class_add(mr_class_t *class, mr_class_t *parent)
{
    size_t mask;
    int ret;

    MR_ASSERT(class != NULL);
    MR_ASSERT(parent != NULL);

    /* Lock the class */
    mask = mr_spin_lock_irqsave(&class->lock);
    mr_spin_lock(&parent->lock);

    /* Check if the same name class already exists */
    if (__class_find(parent, class->name) == NULL)
    {
        ret = MR_EEXIST;
        goto _exit;
    }

    /* Inherit parent class's implementation if current class has no methods */
    if (class->methods == NULL)
    {
        class->privsize = parent->privsize;
        class->methods = parent->methods;

        /* Child class's private data must be larger than parent's */
    } else if (class->privsize < parent->privsize)
    {
        ret = MR_EINVAL;
        goto _exit;
    }

    /* Add the class to the parent */
    class->parent = parent;
    mr_list_append(&parent->clist, &class->list);
    mr_ref_get(&parent->refcount);
    ret = MR_EOK;

_exit:
    /* Unlock the class */
    mr_spin_unlock(&parent->lock);
    mr_spin_unlock_irqrestore(&class->lock, mask);
    return ret;
}

static void __ref_release(struct mr_ref *ref)
{
    mr_class_t *class;

    /* Get the class from the reference */
    class = MR_CONTAINER_OF(ref, mr_class_t, refcount);

    /* Remove the class from the list and release the private data */
    mr_list_remove(&class->list);
    mr_list_remove(&class->clist);
    if (class->privdata != NULL)
    {
        mr_free(class->privdata);
    }

    /* Release the class */
    if (class->release != NULL)
    {
        class->release(class);
    }
}

/**
 * @brief This function delete the class.
 *
 * @param class The class to delete.
 *
 * @note 1.The class will not be visited.
 *       2.If the class reference count is 0, it will be released.
 */
void mr_class_delete(mr_class_t *class)
{
    mr_class_t *parent;
    size_t mask;

    MR_ASSERT(class != NULL);

    /* Lock the class */
    mask = mr_spin_lock_irqsave(&class->lock);
    parent = class->parent;
    if (parent != NULL)
    {
        mr_spin_lock(&parent->lock);
    }

    /* Remove the class */
    mr_list_remove(&class->list);

    /* Unlock the class */
    if (parent != NULL)
    {
        /* Put the reference */
        if (mr_ref_put(&parent->refcount, __ref_release) == false)
        {
            mr_spin_unlock(&parent->lock);
        }
    }

    /* Put the reference */
    if (mr_ref_put(&class->refcount, __ref_release) == true)
    {
        /* Enable interrupt */
        mr_irq_enable(mask);
        return;
    }

    /* Unlock the class if not release */
    mr_spin_unlock_irqrestore(&class->lock, mask);
}

/**
 * @brief This function rename the class.
 *
 * @param class The class to rename.
 * @param name The new name of the class.
 *
 * @return The error code.
 */
int mr_class_rename(mr_class_t *class, const char *name)
{
    mr_class_t *parent;
    size_t mask;

    MR_ASSERT(class != NULL);
    MR_ASSERT(name != NULL);

    /* Lock the class */
    mask = mr_spin_lock_irqsave(&class->lock);
    parent = class->parent;
    if (parent != NULL)
    {
        mr_spin_lock(&parent->lock);

        /* Check if the same name class already exists */
        if (__class_find(parent, name) != NULL)
        {
            /* Unlock the class */
            mr_spin_unlock(&parent->lock);
            mr_spin_unlock_irqrestore(&class->lock, mask);
            return MR_EEXIST;
        }
    }

    /* Rename the class */
    class->name = name;

    /* Unlock the class */
    if (parent != NULL)
    {
        mr_spin_unlock(&parent->lock);
    }
    mr_spin_unlock_irqrestore(&class->lock, mask);
    return MR_EOK;
}

/**
 * @brief This function register the class to the directory.
 *
 * @param class The class to register.
 * @param parent The parent class.
 * @param dir The directory of the class.
 *
 * @return The error code.
 *
 * @note 1.If parent is NULL, path is resolved as an absolute path.
 *       2.If dir is NULL, the class will be registered to the parent.
 */
int mr_class_register(mr_class_t *class, mr_class_t *parent, const char *dir)
{
    MR_ASSERT(class != NULL);

    /* Find the directory class */
    parent = mr_class_find(parent, dir);
    if (parent == NULL)
    {
        return MR_ENOENT;
    }

    /* Add the class to the directory */
    return mr_class_add(class, parent);
}

/**
 * @brief This function unregister the class.
 *
 * @param class The class to unregister.
 *
 * @note 1.The class will not be visited.
 *       2.If the class reference count is 0, it will be released.
 */
void mr_class_unregister(mr_class_t *class)
{
    MR_ASSERT(class != NULL);

    /* Delete the class from the root class */
    mr_class_delete(class);
}

/**
 * @brief This function get the reference of the class.
 *
 * @param class The class.
 *
 * @return The class.
 */
mr_class_t *mr_class_get(mr_class_t *class)
{
    MR_ASSERT(class != NULL);

    /* Get the reference */
    mr_ref_get(&class->refcount);
    return class;
}

/**
 * @brief This function put the reference of the class.
 *
 * @param class The class.
 */
void mr_class_put(mr_class_t *class)
{
    size_t mask;

    MR_ASSERT(class != NULL);

    /* Lock the class */
    mask = mr_spin_lock_irqsave(&class->lock);

    /* Put the reference */
    if (mr_ref_put(&class->refcount, __ref_release) == true)
    {
        /* Enable interrupt */
        mr_irq_enable(mask);
        return;
    }

    /* Unlock the class if not release */
    mr_spin_unlock_irqrestore(&class->lock, mask);
}

/**
 * @brief This function find the class.
 *
 * @param parent The parent class.
 * @param path The path of the class.
 *
 * @return The class.
 *
 * @note If parent is NULL, path is resolved as an absolute path.
 */
mr_class_t *mr_class_find(mr_class_t *parent, const char *path)
{
    mr_class_t *class;
    const char *slash;
    size_t name_size;
    size_t mask;
    char *name;

    /* Check the path */
    if ((path == NULL) || (*path == '\0'))
    {
        return parent;
    }

    /* If no parent, path is an absolute path */
    if (parent == NULL)
    {
        parent = &__root_class;
    }

    /* Skip the leading '/' */
    if (*path == '/')
    {
        path += 1;
        if (*path == '\0')
        {
            return parent;
        }
    }

    /* Get the class name */
    slash = strchr(path, '/');
    name_size = (slash == NULL) ? strlen(path) : (size_t)(slash - path);
    name = mr_malloc(name_size + 1);
    strncpy(name, path, name_size);
    name[name_size] = '\0';

    /* Lock the parent class */
    mask = mr_spin_lock_irqsave(&parent->lock);

    /* Find the class */
    class = __class_find(parent, name);
    mr_free(name);

    /* Unlock the parent class */
    mr_spin_unlock_irqrestore(&parent->lock, mask);

    /* Iterate the directory */
    if (slash != NULL)
    {
        return mr_class_find(class, slash + 1);
    }

    /* Return the found class */
    return class;
}

/**
 * @brief This function check if the class is a subclass of the base class.
 *
 * @param class The class.
 * @param base The base class.
 *
 * @return True if the class is a subclass of the base class, False otherwise.
 */
bool mr_class_is_subclass(mr_class_t *class, mr_class_t *base)
{
    MR_ASSERT(class != NULL);
    MR_ASSERT(base != NULL);

    /* Check if the class is a subclass of the base class */
    while (class != NULL)
    {
        if (class == base)
        {
            return true;
        }
        class = class->parent;
    }
    return false;
}

/**
 * @brief This function get the private data of the class.
 *
 * @param class The class.
 *
 * @return The private data.
 */
void *mr_class_get_privdata(mr_class_t *class)
{
    uint32_t mask;

    MR_ASSERT(class != NULL);

    /* Lock the class */
    mask = mr_spin_lock_irqsave(&class->lock);

    /* Allocate the private data */
    if ((class->privdata == NULL) && (class->privsize > 0))
    {
        class->privdata = mr_malloc(class->privsize);
        memset(class->privdata, 0, class->privsize);
    }

    /* Unlock the class */
    mr_spin_unlock_irqrestore(&class->lock, mask);
    return class->privdata;
}

/**
 * @brief This function get the methods of the class.
 *
 * @param class The class.
 *
 * @return The methods.
 */
void *mr_class_get_methods(mr_class_t *class)
{
    MR_ASSERT(class != NULL);

    /* Get the methods */
    return class->methods;
}

/**
 * @brief This function extract the name from the path.
 *
 * @param path The path to extract.
 *
 * @return The extracted name.
 *
 * @note The name is allocated by mr_malloc, and should be freed by mr_free.
 */
char *mr_extract_name(const char *path)
{
    const char *slash;
    size_t name_size;
    char *name;

    /* Check the path */
    if ((path == NULL) || (*path == '\0'))
    {
        return NULL;
    }

    /* Find the last directory separator in the path */
    slash = strrchr(path, '/');
    if (slash != NULL)
    {
        slash += 1;
        if (*slash != '\0')
        {
            path = slash;
        }
    }

    /* Dealing with "." and ".." as non-names */
    if ((strcmp(path, "..") == 0) || (strcmp(path, ".") == 0))
    {
        return NULL;
    }

    /* Allocate memory for the name */
    name_size = strlen(path);
    name = mr_malloc(name_size + 1);
    if (name == NULL)
    {
        return NULL;
    }

    /* Copy the name to the allocated memory */
    strncpy(name, path, name_size);
    name[name_size] = '\0';
    return name;
}

/**
 * @brief This function extract the directory from the path.
 *
 * @param path The path to extract.
 *
 * @return The extracted directory.
 *
 * @note The directory is allocated by mr_malloc, and should be freed by mr_free.
 */
char *mr_extract_dir(const char *path)
{
    const char *slash;
    size_t dir_size;
    char *dir;

    /* Check the path */
    if (path == NULL)
    {
        return NULL;
    }

    /* Find the last directory separator in the path */
    slash = strrchr(path, '/');
    if (slash == NULL)
    {
        return NULL;
    }

    /* Dealing with "/" "." and ".." as directories */
    if (slash == path)
    {
        slash += 1;
        if (*slash == '\0')
        {
            return NULL;
        }
    } else if (strcmp(slash + 1, "..") == 0)
    {
        slash += 3;
    } else if (strcmp(slash + 1, ".") == 0)
    {
        slash += 2;
    }

    /* Allocate memory for the directory */
    dir_size = slash - path;
    dir = mr_malloc(dir_size + 1);
    if (dir == NULL)
    {
        return NULL;
    }

    /* Copy the directory to the allocated memory */
    strncpy(dir, path, dir_size);
    dir[dir_size] = '\0';
    return dir;
}
