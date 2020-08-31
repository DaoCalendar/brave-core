/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/test/task_environment.h"
#include "bat/ledger/internal/endpoint/payment/post_order/post_order.h"
#include "bat/ledger/internal/ledger_client_mock.h"
#include "bat/ledger/internal/ledger_impl_mock.h"
#include "bat/ledger/ledger.h"
#include "net/http/http_status_code.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=PostOrderTest.*

using ::testing::_;
using ::testing::Invoke;

namespace ledger {
namespace endpoint {
namespace payment {

class PostOrderTest : public testing::Test {
 private:
  base::test::TaskEnvironment scoped_task_environment_;

 protected:
  std::unique_ptr<ledger::MockLedgerClient> mock_ledger_client_;
  std::unique_ptr<bat_ledger::MockLedgerImpl> mock_ledger_impl_;
  std::unique_ptr<PostOrder> order_;

  PostOrderTest() {
    mock_ledger_client_ = std::make_unique<ledger::MockLedgerClient>();
    mock_ledger_impl_ =
        std::make_unique<bat_ledger::MockLedgerImpl>(mock_ledger_client_.get());
    order_ = std::make_unique<PostOrder>(mock_ledger_impl_.get());
  }
};

TEST_F(PostOrderTest, ServerOK) {
  ON_CALL(*mock_ledger_client_, LoadURL(_, _))
      .WillByDefault(
          Invoke([](
              ledger::UrlRequestPtr request,
              ledger::LoadURLCallback callback) {
            ledger::UrlResponse response;
            response.status_code = 201;
            response.url = request->url;
            response.body = R"({
              "id": "f2e6494e-fb21-44d1-90e9-b5408799acd8",
              "createdAt": "2020-06-10T18:58:21.378752Z",
              "currency": "BAT",
              "updatedAt": "2020-06-10T18:58:21.378752Z",
              "totalPrice": "1",
              "merchantId": "",
              "location": "brave.com",
              "status": "pending",
              "items": [
               {
                 "id": "9c9aed7f-b349-452e-80a8-95faf2b1600d",
                 "orderId": "f2e6494e-fb21-44d1-90e9-b5408799acd8",
                 "sku": "user-wallet-vote",
                 "createdAt": "2020-06-10T18:58:21.378752Z",
                 "updatedAt": "2020-06-10T18:58:21.378752Z",
                 "currency": "BAT",
                 "quantity": 4,
                 "price": "0.25",
                 "subtotal": "1",
                 "location": "brave.com",
                 "description": ""
               }
              ]
            })";
            callback(response);
          }));

  ledger::SKUOrderItem item;
  item.quantity = 4;
  item.sku = "asdfasfasfdsdf";
  item.type = ledger::SKUOrderItemType::SINGLE_USE;
  std::vector<ledger::SKUOrderItem> items;
  items.push_back(item);

  order_->Request(
      items,
      [](const ledger::Result result, ledger::SKUOrderPtr order) {
        auto expected_order_item = ledger::SKUOrderItem::New();
        expected_order_item->order_id = "f2e6494e-fb21-44d1-90e9-b5408799acd8";
        expected_order_item->sku = "asdfasfasfdsdf";
        expected_order_item->type = ledger::SKUOrderItemType::SINGLE_USE;
        expected_order_item->order_item_id =
            "9c9aed7f-b349-452e-80a8-95faf2b1600d";
        expected_order_item->quantity = 4;
        expected_order_item->price = 0.25;

        ledger::SKUOrder expected_order;
        expected_order.order_id = "f2e6494e-fb21-44d1-90e9-b5408799acd8";
        expected_order.total_amount = 1;
        expected_order.location = "brave.com";
        expected_order.status = ledger::SKUOrderStatus::PENDING;
        expected_order.items.push_back(std::move(expected_order_item));

        EXPECT_EQ(result, ledger::Result::LEDGER_OK);
        EXPECT_TRUE(expected_order.Equals(*order));
      });
}

TEST_F(PostOrderTest, ServerError400) {
  ON_CALL(*mock_ledger_client_, LoadURL(_, _))
      .WillByDefault(
          Invoke([](
              ledger::UrlRequestPtr request,
              ledger::LoadURLCallback callback) {
            ledger::UrlResponse response;
            response.status_code = 400;
            response.url = request->url;
            response.body = "";
            callback(response);
          }));

  ledger::SKUOrderItem item;
  item.quantity = 4;
  item.sku = "asdfasfasfdsdf";
  item.type = ledger::SKUOrderItemType::SINGLE_USE;
  std::vector<ledger::SKUOrderItem> items;
  items.push_back(item);

  order_->Request(
      items,
      [](const ledger::Result result, ledger::SKUOrderPtr order) {
        EXPECT_EQ(result, ledger::Result::RETRY_SHORT);
        EXPECT_TRUE(!order);
      });
}

TEST_F(PostOrderTest, ServerError500) {
  ON_CALL(*mock_ledger_client_, LoadURL(_, _))
      .WillByDefault(
          Invoke([](
              ledger::UrlRequestPtr request,
              ledger::LoadURLCallback callback) {
            ledger::UrlResponse response;
            response.status_code = 500;
            response.url = request->url;
            response.body = "";
            callback(response);
          }));

  ledger::SKUOrderItem item;
  item.quantity = 4;
  item.sku = "asdfasfasfdsdf";
  item.type = ledger::SKUOrderItemType::SINGLE_USE;
  std::vector<ledger::SKUOrderItem> items;
  items.push_back(item);

  order_->Request(
      items,
      [](const ledger::Result result, ledger::SKUOrderPtr order) {
        EXPECT_EQ(result, ledger::Result::RETRY_SHORT);
        EXPECT_TRUE(!order);
      });
}

TEST_F(PostOrderTest, ServerErrorRandom) {
  ON_CALL(*mock_ledger_client_, LoadURL(_, _))
      .WillByDefault(
          Invoke([](
              ledger::UrlRequestPtr request,
              ledger::LoadURLCallback callback) {
            ledger::UrlResponse response;
            response.status_code = 453;
            response.url = request->url;
            response.body = "";
            callback(response);
          }));

  ledger::SKUOrderItem item;
  item.quantity = 4;
  item.sku = "asdfasfasfdsdf";
  item.type = ledger::SKUOrderItemType::SINGLE_USE;
  std::vector<ledger::SKUOrderItem> items;
  items.push_back(item);

  order_->Request(
      items,
      [](const ledger::Result result, ledger::SKUOrderPtr order) {
        EXPECT_EQ(result, ledger::Result::LEDGER_ERROR);
        EXPECT_TRUE(!order);
      });
}

}  // namespace payment
}  // namespace endpoint
}  // namespace ledger