/** @file
 * Copyright (c) 2019-2020, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/
#ifndef _TEST_S008_ITS_DATA_TESTS_H_
#define _TEST_S008_ITS_DATA_TESTS_H_

#include "val_internal_trusted_storage.h"

#define SST_FUNCTION val->its_function

typedef struct {
    enum its_function_code  api;
    psa_status_t        status;
} test_data;

static const test_data s008_data[] = {
{
 0, 0 /* This is dummy for Index0 */
},
{
 VAL_ITS_SET, PSA_SUCCESS /* Index1 - Create a valid storage entity with zero flag value */
},
{
 VAL_ITS_GET, PSA_SUCCESS /* Index2 - Call get API with offset + data_len = total_size */
},
{
 0, 0 /* This is dummy for Index3 */
},
{
 VAL_ITS_GET, PSA_SUCCESS /* Index4 - Call get API with offset + data_len < total_size */
},
{
 0, 0 /* This is dummy for Index5 */
},
{
 VAL_ITS_GET, PSA_SUCCESS /* Index6 - Call get API with offset = total data_size + 1 */
},
{
 0, 0 /* This is dummy for Index7 */
},
{
 VAL_ITS_GET, PSA_SUCCESS /* Index8 - get API with offset = total data_size */
},
{
 0, 0 /* This is dummy for Index9 */
},
{
 VAL_ITS_GET, PSA_SUCCESS /* Index10 - Call get API with invalid data len and offset zero */
},
{
 0, 0 /* This is dummy for Index11 */
},
{
 VAL_ITS_GET, PSA_ERROR_INVALID_ARGUMENT /* Index12 - Call get API with offset = MAX_UINT32 */
},
{
 VAL_ITS_REMOVE, PSA_SUCCESS /* Index13 - Remove the storage entity */
},
};
#endif /* _TEST_S008_ITS_DATA_TESTS_H_ */
