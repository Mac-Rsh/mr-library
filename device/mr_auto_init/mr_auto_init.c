/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-02-01     MacRsh       first version
 */

#include "mr_auto_init.h"

static int mri_start(void)
{
  return 0;
}
INIT_EXPORT(mri_start, "0");

static int mri_board_start(void)
{
  return 0;
}
INIT_EXPORT(mri_board_start, "0.end");

static int mri_board_end(void)
{
  return 0;
}
INIT_EXPORT(mri_board_end, "1.end");

static int mri_end(void)
{
  return 0;
}
INIT_EXPORT(mri_end,"6.end");

void mr_auto_init(void)
{
  volatile const init_fn_t *fn_ptr;

  /* auto-init-board */
  for (fn_ptr = &_mr_init_mri_start; fn_ptr < &_mr_init_mri_board_end; fn_ptr++)
  {
    (*fn_ptr)();
  }

  /* auto-init-other */
  for (fn_ptr = &_mr_init_mri_board_end; fn_ptr < &_mr_init_mri_end; fn_ptr++)
  {
    (*fn_ptr)();
  }
}



