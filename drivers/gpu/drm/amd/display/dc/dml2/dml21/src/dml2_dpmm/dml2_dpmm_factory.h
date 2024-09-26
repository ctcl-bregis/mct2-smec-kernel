// SPDX-License-Identifier: MIT
//
// Copyright 2024 Advanced Micro Devices, Inc.


#ifndef __DML2_DPMM_FACTORY_H__
#define __DML2_DPMM_FACTORY_H__

#include "dml2_internal_shared_types.h"
#include "dml_top_types.h"

bool dml2_dpmm_create(enum dml2_project_id project_id, struct dml2_dpmm_instance *out);

#endif
