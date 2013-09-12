<?php

/**
* Implements hook_form_FORMID_alter() to change the Add to Cart button to an image.
*/
function wv_form_commerce_cart_add_to_cart_form_alter(&$form, &$form_state) {
  //dpm($form);
  //dpm($form_state['line_item']);
  $line_item = $form_state['line_item'];
  $product = commerce_product_load($line_item->commerce_product[LANGUAGE_NONE][0]['product_id']);
  //dpm($product);
  $form['submit']['#value'] = t('Give Now');
  $form['submit']['#attributes']['class'][] = 'give-now';
  if ($product->type == 'needed_most') {
    $form['submit']['#value'] = t('Add Donation');
    $form['submit']['#attributes']['class'] = array('form-submit');
    $form['line_item_fields']['field_simple_donation'][LANGUAGE_NONE]['#title'] = '';
    $form['line_item_fields']['field_simple_donation'][LANGUAGE_NONE][0]['#title'] = '';
  }
  if ($product->type == 'donation_product') {
    // Make it look less crappy
    $form['line_item_fields']['field_donation2'][LANGUAGE_NONE]['#title'] = '';

    // Find all products, add it to the "select or other"
    $form['line_item_fields']['field_donation2'][LANGUAGE_NONE]['#options'] = array();
    //dpm($form);
    if (@$form['product_id']['#options']) {
      foreach ($form['product_id']['#options'] as $product_ref => $uselesstitle) {
        $product_loaded = commerce_product_load($product_ref);
        $form['line_item_fields']['field_donation2'][LANGUAGE_NONE]['#options'][substr($product_loaded->commerce_price['und'][0]['amount'],0,-2)] = substr($product_loaded->commerce_price['und'][0]['amount'],0,-2).'.00';
      }
    } elseif (@$form['product_id']['#value']) {
        $product_loaded = commerce_product_load($form['product_id']['#value']);
        $form['line_item_fields']['field_donation2'][LANGUAGE_NONE]['#options'][substr($product_loaded->commerce_price['und'][0]['amount'],0,-2)] = substr($product_loaded->commerce_price['und'][0]['amount'],0,-2).'.00';
    }

    // Reset order
    reset($form['line_item_fields']['field_donation2'][LANGUAGE_NONE]['#options']);
    // Pull top id
    $first_key = key($form['line_item_fields']['field_donation2'][LANGUAGE_NONE]['#options']);
    $default_value = $form['line_item_fields']['field_donation2'][LANGUAGE_NONE]['#options'][$first_key];
    $form['line_item_fields']['field_donation2'][LANGUAGE_NONE]['#default_value'] = $default_value;
  }
  if ($product->type == 'recurring') {
    if (@$form['product_id']['#value']) {
      $product_loaded = commerce_product_load($form['product_id']['#value']);
      $form['before']['#markup'] = '<p><strong>Monthly Sponsorship: $'.substr($product_loaded->commerce_price['und'][0]['amount'],0,-2).'</strong></p>';
      $form['submit']['#value'] = t('Sponsor '.$product_loaded->title.' Today!');
    }
  }
  if ($product->type == 'micro_loan') {
    // Full Loans
    $full_loan = substr($product->commerce_price['und'][0]['amount'],0,-2);
    $amount_purchased = _wv_product_purchased_summedup($product->product_id);
    $percent_left = ($amount_purchased["total"]/$full_loan)*100;
    $left_to_purchase = $full_loan - $amount_purchased["total"];
    $form['line_item_fields']['field_micro_loan_donation'][LANGUAGE_NONE]['#options'] = array();
    unset($form['line_item_fields']['field_micro_loan_donation'][LANGUAGE_NONE]['#title']);
    if ($left_to_purchase > 100) {
      foreach (range(0,100,25) as $donation_amount) {
        $form['line_item_fields']['field_micro_loan_donation'][LANGUAGE_NONE]['#options'][$donation_amount] = '$'.$donation_amount;
      }
      foreach (range(100,$left_to_purchase,100) as $donation_amount) {
        $form['line_item_fields']['field_micro_loan_donation'][LANGUAGE_NONE]['#options'][$donation_amount] = '$'.$donation_amount;
      }
      $form['line_item_fields']['field_micro_loan_donation'][LANGUAGE_NONE]['#options'][$left_to_purchase] = '$'.$left_to_purchase;
    } else {
      foreach (range(25,$left_to_purchase,25) as $donation_amount) {
        $form['line_item_fields']['field_micro_loan_donation'][LANGUAGE_NONE]['#options'][$donation_amount] = '$'.$donation_amount;
      }
    }
    $form['thermometer']['#markup'] = '<div class="microloan">
    <div class="numbers">
      <div class="full"><span>$'.$full_loan.'</span>Loan Amount</div>
      <div class="needed"><span>$'.$left_to_purchase.'</span>Still Neeeded</div>
      <div class="raised"><span>'.round($percent_left,0).'%</span>Loan Raised</div>
      <div class="lenders"><span>'.$amount_purchased["lenders"].'</span>Lenders</div>
    </div>
    <div class="gauge-Small"><div class="current-value" id="campaign-progress-current" style="height: '.$percent_left.'%;"><p>'.$percent_left.'%</p></div></div></div>';
    $form['submit']['#value'] = t('Donate');
  }
}


function _wv_product_purchased_summedup($pid) {
  $product_ids = array();
  foreach (commerce_order_load_multiple(array(), array('status'=>'pending'), TRUE) as $order) {
    foreach (entity_metadata_wrapper('commerce_order', $order)->commerce_line_items as $delta => $line_item_wrapper) {
      if (in_array($line_item_wrapper->type->value(), commerce_product_line_item_types())) {
        $id = $line_item_wrapper->commerce_product->product_id->raw();
        $amount = substr($line_item_wrapper->commerce_total->amount->raw(),0,-2);
        $product_ids[$id]["total"] += $amount;
        if (@$product_ids[$id]["lenders"]) {
          $product_ids[$id]["lenders"]++;
        } else {
          $product_ids[$id]["lenders"] = 1;
        }
      }
    }
  }
  if (@$product_ids[$pid]) {
    return $product_ids[$pid];
  } else {
    return array("total"=>0,"lenders"=>0);
  }
}

/**
 * Preprocess variables for html.tpl.php
 *
 * @see system_elements()
 * @see html.tpl.php
 */
function wv_theme_preprocess_html(&$variables) {
  // Add conditional stylesheets for IE
  drupal_add_css(path_to_theme() . '/css/commerce-kickstart-theme-ie-lte-8.css', array('group' => CSS_THEME, 'weight' => 23, 'browsers' => array('IE' => 'lte IE 8', '!IE' => FALSE), 'preprocess' => FALSE));
  drupal_add_css(path_to_theme() . '/css/commerce-kickstart-theme-ie-lte-7.css', array('group' => CSS_THEME, 'weight' => 24, 'browsers' => array('IE' => 'lte IE 7', '!IE' => FALSE), 'preprocess' => FALSE));

  // Add external libraries.
  drupal_add_library('commerce_kickstart_theme', 'selectnav');
}

/**
 * Implements hook_library().
 */
function wv_theme_library() {
  $libraries['selectnav'] = array(
    'title' => 'Selectnav',
    'version' => '',
    'js' => array(
      libraries_get_path('selectnav.js') . '/selectnav.min.js' => array(),
    ),
  );
  return $libraries;
}

/**
 * Override the submitted variable.
 */
function wv_theme_preprocess_node(&$variables) {
  $variables['submitted'] = $variables['date'] . ' - ' . $variables['name'];
  if ($variables['type'] == 'blog_post') {
    $variables['submitted'] = t('By') . ' ' . $variables['name'] . ', ' . $variables['date'];
  }
}

//menu_rebuild();
function wv_menu_alter(&$items) {
  $items['cart']['title'] = t('Gifts from the Heart');
  dpm($items);
}
