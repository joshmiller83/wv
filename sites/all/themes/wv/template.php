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
