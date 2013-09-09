<?php

/**
 * @file
 * Default Rules configuration.
 */

/**
 * Implements hook_default_rules_configuration().
 */
function commerce_dunning_defaults_default_rules_configuration() {
  $items = array();

  // Component: Rule: Send dunning notification.
  $items['rules_commerce_dunning_send_notification'] = entity_import('rules_config', '{ "rules_commerce_dunning_send_notification" : {
      "LABEL" : "Send dunning notification",
      "PLUGIN" : "rule",
      "TAGS" : [ "Commerce Dunning" ],
      "REQUIRES" : [ "rules", "message_notify" ],
      "USES VARIABLES" : {
        "message" : { "label" : "Message", "type" : "message" },
        "order" : { "label" : "Order", "type" : "commerce_order" }
      },
      "IF" : [
        { "entity_has_field" : { "entity" : [ "message" ], "field" : "commerce_dunning_message_order" } }
      ],
      "DO" : [
        { "data_set" : {
            "data" : [ "message:commerce_dunning_message_order" ],
            "value" : [ "order" ]
          }
        },
        { "entity_save" : { "data" : [ "message" ], "immediate" : 1 } },
        { "message_notify_process" : { "message" : [ "message" ] } }
      ]
    }
  }');

  // Component: Rule: Send notification that card data needs to be updated.
  $items['rules_commerce_dunning_send_notification_card_data_needs_update'] = entity_import('rules_config', '{ "rules_commerce_dunning_send_notification_card_data_needs_update" : {
      "LABEL" : "Send notification that card data needs to be updated",
      "PLUGIN" : "rule",
      "TAGS" : [ "Commerce Dunning" ],
      "REQUIRES" : [ "commerce_dunning", "rules" ],
      "USES VARIABLES" : { "order" : { "label" : "Order", "type" : "commerce_order" } },
      "IF" : [
        { "NOT commerce_dunning_cardonfile_status" : { "order" : [ "order" ], "status" : "1" } }
      ],
      "DO" : [
        { "entity_create" : {
            "USING" : {
              "type" : "message",
              "param_type" : "commerce_dunning_card_data_needs_update",
              "param_user" : [ "order:owner" ]
            },
            "PROVIDE" : { "entity_created" : { "message_entity_created" : "Created message entity" } }
          }
        },
        { "component_rules_commerce_dunning_send_notification" : { "message" : [ "message-entity-created" ], "order" : [ "order" ] } }
      ]
    }
  }');

  // Reaction rule: Notify customer about a hard declined transaction.
  $items['rules_commerce_dunning_send_notification_hard_declined'] = entity_import('rules_config', '{ "rules_commerce_dunning_send_notification_hard_declined" : {
      "LABEL" : "Notify customer about a hard declined transaction",
      "PLUGIN" : "reaction rule",
      "TAGS" : [ "Commerce Dunning" ],
      "REQUIRES" : [ "rules", "commerce_cardonfile" ],
      "ON" : [ "commerce_cardonfile_charge_failed" ],
      "IF" : [
        { "data_is" : {
            "data" : [ "response:code" ],
            "op" : "IN",
            "value" : { "value" : {
                "insufficient" : "insufficient",
                "card_na" : "card_na",
                "card_not_chargeable" : "card_not_chargeable",
                "no_method" : "no_method",
                "method_not_capable" : "method_not_capable",
                "card_expired" : "card_expired",
                "method_failure_hard_decline" : "method_failure_hard_decline"
              }
            }
          }
        }
      ],
      "DO" : [
        { "entity_create" : {
            "USING" : {
              "type" : "message",
              "param_type" : "commerce_dunning_hard_declined_card",
              "param_user" : [ "order:owner" ]
            },
            "PROVIDE" : { "entity_created" : { "message_entity_created" : "Created message entity" } }
          }
        },
        { "component_rules_commerce_dunning_send_notification" : { "message" : [ "message-entity-created" ], "order" : [ "order" ] } },
        { "data_set" : { "data" : [ "response:card-chosen:status" ], "value" : "3" } }
      ]
    }
  }');

  // Reaction rule: Notify customer about a soft declined transaction.
  $items['rules_commerce_dunning_send_notification_soft_declined'] = entity_import('rules_config', '{ "rules_commerce_dunning_send_notification_soft_declined" : {
      "LABEL" : "Notify customer about a soft declined transaction",
      "PLUGIN" : "reaction rule",
      "TAGS" : [ "Commerce Dunning" ],
      "REQUIRES" : [ "rules", "commerce_cardonfile" ],
      "ON" : [ "commerce_cardonfile_charge_failed" ],
      "IF" : [
        { "data_is" : {
            "data" : [ "response:code" ],
            "op" : "IN",
            "value" : { "value" : {
                "method_failure" : "method_failure",
                "method_failure_insufficient_funds" : "method_failure_insufficient_funds",
                "method_failure_limit_exceeded" : "method_failure_limit_exceeded",
                "method_failure_call_issuer" : "method_failure_call_issuer",
                "method_failure_temporary_hold" : "method_failure_temporary_hold",
                "method_failure_generic_decline" : "method_failure_generic_decline",
                "method_failure_gateway_error" : "method_failure_gateway_error",
                "method_failure_gateway_unavailable" : "method_failure_gateway_unavailable",
                "method_failure_gateway_configuration_error" : "method_failure_gateway_configuration_error"
              }
            }
          }
        }
      ],
      "DO" : [
        { "entity_create" : {
            "USING" : {
              "type" : "message",
              "param_type" : "commerce_dunning_soft_declined_card",
              "param_user" : [ "order:owner" ]
            },
            "PROVIDE" : { "entity_created" : { "message_entity_created" : "Created message entity" } }
          }
        },
        { "component_rules_commerce_dunning_send_notification" : { "message" : [ "message-entity-created" ], "order" : [ "order" ] } }
      ]
    }
  }');

  // Reaction rule: Schedule charge attempts.
  $items['rules_commerce_dunning_schedule_charge_attempts'] = entity_import('rules_config', '{ "rules_commerce_dunning_schedule_charge_attempts" : {
      "LABEL" : "Schedule charge attempts",
      "PLUGIN" : "reaction rule",
      "TAGS" : [ "Commerce Dunning" ],
      "REQUIRES" : [ "commerce_dunning", "rules", "rules_scheduler", "commerce_cardonfile" ],
      "ON" : [ "commerce_cardonfile_charge_failed" ],
      "IF" : [
        { "NOT commerce_dunning_order_revision_status" : {
            "order" : [ "order" ],
            "status" : { "value" : {
                "cardonfile_payment_error_hard_decline" : "cardonfile_payment_error_hard_decline",
                "cardonfile_payment_failed_soft_decline" : "cardonfile_payment_failed_soft_decline"
              }
            }
          }
        }
      ],
      "DO" : [
        { "schedule" : {
            "component" : "rules_commerce_dunning_charge_card",
            "date" : "+3 days",
            "identifier" : "dunning-[order:order-id]-charge-attempt-3-days",
            "param_order" : [ "order" ]
          }
        },
        { "schedule" : {
            "component" : "rules_commerce_dunning_charge_card",
            "date" : "+6 days",
            "identifier" : "dunning-[order:order-id]-charge-attempt-6-days",
            "param_order" : [ "order" ]
          }
        },
        { "schedule" : {
            "component" : "rules_commerce_dunning_charge_card",
            "date" : "+9 days",
            "identifier" : "dunning-[order:order-id]-charge-attempt-9-days",
            "param_order" : [ "order" ]
          }
        },
        { "schedule" : {
            "component" : "rules_commerce_dunning_charge_card",
            "date" : "+12 days",
            "identifier" : "dunning-[order:order-id]-charge-attempt-12-days",
            "param_order" : [ "order" ]
          }
        },
        { "schedule" : {
            "component" : "rules_commerce_dunning_terminate_dunning_cycle",
            "date" : "+15 days",
            "identifier" : "dunning-[order:order-id]-end-15-days",
            "param_order" : [ "order" ]
          }
        }
      ]
    }
  }');

  // Reaction rule: Schedule hard decline notifications.
  $items['rules_commerce_dunning_schedule_hard_decline_notifications'] = entity_import('rules_config', '{ "rules_commerce_dunning_schedule_hard_decline_notifications" : {
      "LABEL" : "Schedule hard decline notifications",
      "PLUGIN" : "reaction rule",
      "TAGS" : [ "Commerce Dunning" ],
      "REQUIRES" : [ "rules", "commerce_dunning", "rules_scheduler", "commerce_cardonfile" ],
      "ON" : [ "commerce_cardonfile_charge_failed" ],
      "IF" : [
        { "data_is" : {
            "data" : [ "response:code" ],
            "op" : "IN",
            "value" : { "value" : {
                "insufficient" : "insufficient",
                "card_na" : "card_na",
                "card_not_chargeable" : "card_not_chargeable",
                "no_method" : "no_method",
                "method_not_capable" : "method_not_capable",
                "card_expired" : "card_expired",
                "method_failure_hard_decline" : "method_failure_hard_decline"
              }
            }
          }
        },
        { "NOT commerce_dunning_order_revision_status" : {
            "order" : [ "order" ],
            "status" : { "value" : { "cardonfile_payment_error_hard_decline" : "cardonfile_payment_error_hard_decline" } }
          }
        }
      ],
      "DO" : [
        { "schedule" : {
            "component" : "rules_commerce_dunning_send_notification_card_data_needs_update",
            "date" : "+2 days",
            "identifier" : "dunning-[order:order-id]-hard-decline-notification-2-days",
            "param_order" : [ "order" ]
          }
        },
        { "schedule" : {
            "component" : "rules_commerce_dunning_send_notification_card_data_needs_update",
            "date" : "+5 days",
            "identifier" : "dunning-[order:order-id]-hard-decline-notification-5-days",
            "param_order" : [ "order" ]
          }
        },
        { "schedule" : {
            "component" : "rules_commerce_dunning_send_notification_card_data_needs_update",
            "date" : "+8 days",
            "identifier" : "dunning-[order:order-id]-hard-decline-notification-8-days",
            "param_order" : [ "order" ]
          }
        },
        { "schedule" : {
            "component" : "rules_commerce_dunning_send_notification_card_data_needs_update",
            "date" : "+11 days",
            "identifier" : "dunning-[order:order-id]-hard-decline-notification-11-days",
            "param_order" : [ "order" ]
          }
        }
      ]
    }
  }');

  // Reaction rule: Unschedule all further dunning steps.
  $items['rules_commerce_dunning_unschedule_all_further_steps'] = entity_import('rules_config', '{ "rules_commerce_dunning_unschedule_all_further_steps" : {
      "LABEL" : "Unschedule all further dunning steps",
      "PLUGIN" : "reaction rule",
      "TAGS" : [ "Commerce Dunning" ],
      "REQUIRES" : [ "commerce_payment", "commerce_dunning", "commerce_cardonfile" ],
      "ON" : [ "commerce_cardonfile_charge_success" ],
      "IF" : [
        { "commerce_payment_order_balance_comparison" : { "commerce_order" : [ "order" ], "value" : "0" } }
      ],
      "DO" : [ { "commerce_dunning_unschedule_tasks" : { "order" : [ "order" ] } } ]
    }
  }');

  return $items;
}
