#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "db_access.h"

using ::testing::Return;
using ::testing::_;

class MockResult : public PGResultInterface {
public:
  MOCK_CONST_METHOD0(status, ExecStatusType());
  MOCK_CONST_METHOD0(ntuples, int());
  MOCK_CONST_METHOD2(value, char*(int, int));
  MOCK_METHOD0(clear, void());
};

class MockConn : public PGConnInterface {
public:
  MOCK_CONST_METHOD0(status, ExecStatusType());
  MOCK_CONST_METHOD0(errorMessage, const char*());
  MOCK_METHOD1(exec, std::unique_ptr<PGResultInterface>(const char*));
  MOCK_METHOD0(finish, void());
};

TEST(DatabaseTest, ReturnsEventsSuccessfully) {
  auto mockRes = new MockResult();
  auto mockConnPtr = std::make_unique<MockConn>();
  MockConn* mockConn = mockConnPtr.get();

  EXPECT_CALL(*mockConn, status()).WillOnce(Return(CONNECTION_OK));
  EXPECT_CALL(*mockConn, exec(_)).WillOnce(
    Return(std::unique_ptr<PGResultInterface>(mockRes)));
  EXPECT_CALL(*mockRes, status()).WillOnce(Return(PGRES_TUPLES_OK));
  EXPECT_CALL(*mockRes, ntuples()).WillOnce(Return(1));
  EXPECT_CALL(*mockRes, value(0,0)).WillOnce(Return("id1"));
  EXPECT_CALL(*mockRes, value(0,1)).WillOnce(Return("42"));
  EXPECT_CALL(*mockRes, value(0,2)).WillOnce(Return("2025-07-20 14:00:00"));
  EXPECT_CALL(*mockRes, clear());
  EXPECT_CALL(*mockConn, finish());

  Database db(std::move(mockConnPtr));
  auto events = db.getRecentSensorEvents();

  ASSERT_EQ(events.size(), 1);
  EXPECT_EQ(events[0].sensor_id, "id1");
  EXPECT_EQ(events[0].value, "42");
}

TEST(DatabaseTest, ThrowsOnConnectionFailure) {
  auto mockConnPtr = std::make_unique<MockConn>();
  MockConn* mockConn = mockConnPtr.get();

  EXPECT_CALL(*mockConn, status()).WillOnce(Return(CONNECTION_BAD));
  EXPECT_CALL(*mockConn, errorMessage()).WillOnce(Return("fail"));
  EXPECT_CALL(*mockConn, finish());

  Database db(std::move(mockConnPtr));
  EXPECT_THROW(db.getRecentSensorEvents(), std::runtime_error);
}