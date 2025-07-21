#include <gtest/gtest.h>
#include "db_access.h"   // contains Database class & getRecentSensorEvents
#include <chrono>

class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Optionally set up test DB or verify connection before tests
        // Make sure your DB "sensordb" with table sensor_events is ready
    }

    void TearDown() override {
        // Clean up after tests if needed
    }
};

TEST_F(DatabaseTest, GetRecentSensorEventsReturnsAtMost10Events) {
    Database db;
    auto events = db.getRecentSensorEvents();
    ASSERT_LE(events.size(), 10);
}

TEST_F(DatabaseTest, GetRecentSensorEventsHasValidSensorIdAndValue) {
    Database db;
    auto events = db.getRecentSensorEvents();
    for (const auto& e : events) {
        EXPECT_FALSE(e.sensor_id.empty());
        EXPECT_FALSE(e.value.empty());
    }
}

TEST_F(DatabaseTest, GetRecentSensorEventsHasValidTimestamps) {
    Database db;
    auto events = db.getRecentSensorEvents();
    for (const auto& e : events) {
        // Check timestamps are reasonable (not default time)
        // e.g., greater than epoch start or some date like 2000-01-01
        auto tp = e.timestamp;
        std::time_t t = std::chrono::system_clock::to_time_t(tp);
        EXPECT_GT(t, 946684800); // 2000-01-01 00:00:00 UTC
    }
}