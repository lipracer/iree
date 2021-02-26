// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "iree/base/status.h"

#include "iree/testing/gtest.h"
#include "iree/testing/status_matchers.h"

namespace iree {
namespace {

using ::iree::testing::status::StatusIs;
using ::testing::HasSubstr;

#if (IREE_STATUS_FEATURES & IREE_STATUS_FEATURE_ANNOTATIONS) != 0
#define CHECK_STATUS_MESSAGE(status, message_substr)         \
  EXPECT_THAT(status.ToString(),                             \
              HasSubstr(StatusCodeToString(status.code()))); \
  EXPECT_THAT(status.ToString(), HasSubstr(message_substr))
#define CHECK_STREAM_MESSAGE(status, os, message_substr)               \
  EXPECT_THAT(os.str(), HasSubstr(StatusCodeToString(status.code()))); \
  EXPECT_THAT(os.str(), HasSubstr(message_substr))
#else
#define CHECK_STATUS_MESSAGE(status, message_substr) \
  EXPECT_THAT(status.ToString(), HasSubstr(StatusCodeToString(status.code())));
#define CHECK_STREAM_MESSAGE(status, os, message_substr) \
  EXPECT_THAT(os.str(), HasSubstr(StatusCodeToString(status.code())));
#endif  // has IREE_STATUS_FEATURE_ANNOTATIONS

TEST(Status, ConstructedWithMessage) {
  Status status = Status(StatusCode::kInvalidArgument, "message");
  CHECK_STATUS_MESSAGE(status, "message");
}

TEST(Status, StreamInsertion) {
  Status status = Status(StatusCode::kInvalidArgument, "message");
  std::ostringstream os;
  os << status;
  CHECK_STREAM_MESSAGE(status, os, "message");
}

TEST(Status, StreamInsertionContinued) {
  Status status = Status(StatusCode::kInvalidArgument, "message");
  std::ostringstream os;
  os << status << " annotation";
  CHECK_STREAM_MESSAGE(status, os, "message");
  CHECK_STREAM_MESSAGE(status, os, "annotation");
}

TEST(StatusMacro, ReturnIfError) {
  auto returnIfError = [](iree_status_t status) -> iree_status_t {
    IREE_RETURN_IF_ERROR(status, "annotation");
    return iree_ok_status();
  };
  Status status = iree_make_status(IREE_STATUS_INVALID_ARGUMENT, "message");
  status = returnIfError(std::move(status));
  EXPECT_THAT(status, StatusIs(StatusCode::kInvalidArgument));
  CHECK_STATUS_MESSAGE(status, "message");
  CHECK_STATUS_MESSAGE(status, "annotation");

  IREE_EXPECT_OK(returnIfError(OkStatus()));
}

TEST(StatusMacro, ReturnIfErrorFormat) {
  auto returnIfError = [](iree_status_t status) -> iree_status_t {
    IREE_RETURN_IF_ERROR(status, "annotation %d %d %d", 1, 2, 3);
    return iree_ok_status();
  };
  Status status = iree_make_status(IREE_STATUS_INVALID_ARGUMENT, "message");
  status = returnIfError(std::move(status));
  EXPECT_THAT(status, StatusIs(StatusCode::kInvalidArgument));
  CHECK_STATUS_MESSAGE(status, "message");
  CHECK_STATUS_MESSAGE(status, "annotation 1 2 3");

  IREE_EXPECT_OK(returnIfError(OkStatus()));
}

TEST(StatusMacro, AssignOrReturn) {
  auto assignOrReturn = [](StatusOr<std::string> statusOr) -> iree_status_t {
    IREE_ASSIGN_OR_RETURN(auto ret, std::move(statusOr));
    (void)ret;
    return iree_ok_status();
  };
  StatusOr<std::string> statusOr =
      iree_make_status(IREE_STATUS_INVALID_ARGUMENT, "message");
  Status status = assignOrReturn(std::move(statusOr));
  EXPECT_THAT(status, StatusIs(StatusCode::kInvalidArgument));
  CHECK_STATUS_MESSAGE(status, "message");

  IREE_EXPECT_OK(assignOrReturn("foo"));
}

}  // namespace
}  // namespace iree