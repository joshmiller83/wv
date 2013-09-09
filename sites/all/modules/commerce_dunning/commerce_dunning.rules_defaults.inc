<?php

/**
 * @file
 * Default Rules configuration.
 */

/**
 * Implements hook_default_rules_configuration().
 */
function commerce_dunning_default_rules_configuration() {
  $items = array();

  // Component: Rule: Charge card.
  $items['rules_commerce_dunning_charge_card'] = entity_import('rules_config', '{ "rules_commerce_dunning_charge_card" : {
      "LABEL" : "Charge card",
      "PLUGIN" : "rule",
      "TAGS" : [ "Commerce Dunning" ],
      "REQUIRES" : [ "commerce_payment", "commerce_dunning", "commerce_cardonfile" ],
      "USES VARIABLES" : { "order" : { "label" : "Order", "type" : "commerce_order" } },
      "IF" : [
        { "commerce_payment_order_balance_comparison" : { "commerce_order" : [ "order" ], "operator" : "\u003E", "value" : "0" } },
        { "commerce_dunning_cardonfile_status" : { "order" : [ "order" ], "status" : "1" } }
      ],
      "DO" : [
        { "commerce_cardonfile_order_select_card" : {
            "USING" : { "order" : [ "order" ] },
            "PROVIDE" : { "select_card_response" : { "select_card_response" : "Select card response" } }
          }
        },
        { "commerce_cardonfile_order_charge_card" : {
            "USING" : {
              "order" : [ "order" ],
              "charge" : [ "" ],
              "select_card_response" : [ "select-card-response" ],
              "card_data" : [ "" ]
            },
            "PROVIDE" : { "charge_card_response" : { "charge_card_response" : "Charge Card Response" } }
          }
        }
      ]
    }
  }');

  // Component: Action set: Terminate dunning cycle.
  $items['rules_commerce_dunning_terminate_dunning_cycle'] = entity_import('rules_config', '{ "rules_commerce_dunning_terminate_dunning_cycle" : {
      "LABEL" : "Terminate dunning cycle",
      "PLUGIN" : "action set",
      "TAGS" : [ "Commerce Dunning" ],
      "REQUIRES" : [ "commerce_dunning", "rules" ],
      "USES VARIABLES" : { "order" : { "label" : "Order", "type" : "commerce_order" } },
      "ACTION SET" : [
        { "commerce_dunning_invoke_dunning_end" : { "order" : [ "order" ] } },
        { "data_set" : { "data" : [ "order:status" ], "value" : "dunning_cycle_terminated" } },
        { "commerce_dunning_unschedule_tasks" : { "order" : [ "order" ] } }
      ]
    }
  }');

  return $items;
}
