<?php

/**
 * @file
 * User page callbacks and forms for Commerce Card on File.
 */

/**
 * Menu item title callback: returns a Card Title for its pages
 * comprised of the card type and last 4 digits.
 *
 * @param $card
 *   The card object as loaded via the URL wildcard.
 * @return
 *   A page title of the format "Card ##".
 */
function commerce_cardonfile_card_title($card_data) {
  return t('@type ending in @number', array('@type' => $card_data->card_type, '@number' => $card_data->card_number));
}

/**
 * Displays the full details of a Card on file.
 */
function commerce_cardonfile_card_view($card_data, $view_mode) {
  return entity_view('commerce_cardonfile', array($card_data->card_id => $card_data), $view_mode, NULL, TRUE);
}


/**
 * Displays the form for updating cardonfile data.
 *
 * @param object $card_data
 *   The card on file entity.
 */
function commerce_cardonfile_update($card_data) {
  // Set the title of the page.
  drupal_set_title(t('Update your @type ending in @number', array('@type' => $card_data->card_type, '@number' => $card_data->card_number)));
  $payment_method = commerce_payment_method_instance_load($card_data->instance_id);

  if ($form_calback = commerce_cardonfile_payment_method_callback($payment_method, 'update form callback')) {
    return drupal_get_form($form_calback, $card_data);
  }
  else {
    return drupal_get_form('commerce_cardonfile_update_form', $card_data);
  }
}

/**
 * Builds the form for updating cardonfile data.
 *
 */
function commerce_cardonfile_update_form($form, &$form_state, $card_data) {
  // Load the credit card helper functions from the Payment module.
  module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');

  $form['card_data'] = array(
    '#type' => 'value',
    '#value' => $card_data,
  );

  $defaults = array(
    'exp_month' => $card_data->card_exp_month,
    'exp_year' => $card_data->card_exp_year,
  );

  $form += commerce_payment_credit_card_form(array(), $defaults);

  $form['credit_card']['card_holder'] = array(
    '#type' => 'item',
    '#title' => t('Card holder'),
    '#markup' => $card_data->card_name,
    '#weight' => -5,
  );

  $form['credit_card']['number'] = array(
    '#type' => 'item',
    '#title' => t('Card Number'),
    '#markup' => 'XXXX - XXXX - XXXX - ' . $card_data->card_number,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update card data'),
    '#suffix' => l(t('Cancel'), 'user/' . $card_data->uid . '/cards'),
  );

  return $form;
}

/**
 * Form validate handler: update stored card data.
 */
function commerce_cardonfile_update_form_validate($form, &$form_state) {
  module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');

  // Ensure the expiration date is not being updated to a past date.
  $exp_valid = commerce_payment_validate_credit_card_exp_date($form_state['values']['credit_card']['exp_month'], $form_state['values']['credit_card']['exp_year']);

  if ($exp_valid !== TRUE) {
    form_set_error('credit_card][exp_' . $exp_valid, t('You have specified an expired credit card.'));
  }
}

/**
 * Form submit handler: update stored card data.
 */
function commerce_cardonfile_update_form_submit($form, &$form_state) {
  // Update the card data with items from the form.
  $card_data = $form_state['values']['card_data'];

  $card_data->card_exp_month = $form_state['values']['credit_card']['exp_month'];
  $card_data->card_exp_year = $form_state['values']['credit_card']['exp_year'];

  // Invoke the payment method's card update callback.
  $payment_method = commerce_payment_method_instance_load($card_data->instance_id);
  $callback = $payment_method['cardonfile']['update callback'];

  if (function_exists($callback)) {
    if (!$callback($form, $form_state, $payment_method, $card_data)) {
      // Display a message if we failed to communicate properly with the payment
      // gateway in the Card on File update callback.
      drupal_set_message(t('We encountered an error attempting to update your card data. Please try again and contact us if this error persists.'), 'error');
      $form_state['rebuild'] = TRUE;
      return;
    }
  }

  commerce_cardonfile_save($card_data);

  drupal_set_message(t('The stored card data has been updated.'));

  $form_state['redirect'] = 'user/' . $card_data->uid . '/cards';
}

/**
 * Builds the form for deleting cardonfile data.
 *
 */
function commerce_cardonfile_delete_form($form, &$form_state, $card_data) {
  module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');

  $card_types = commerce_payment_credit_card_types();

  $form['card_type'] = array(
    '#type' => 'item',
    '#title' => t('Card type'),
    '#markup' => !empty($card_types[$card_data->card_type]) ? $card_types[$card_data->card_type] : $card_data->card_type,
  );

  $form['card_number'] = array(
    '#type' => 'item',
    '#title' => t('Card number'),
    '#markup' => 'XXXX - XXXX - XXXX - ' . $card_data->card_number,
  );

  $form['card_expire'] = array(
    '#type' => 'item',
    '#title' => t('Expires'),
    '#markup' => $card_data->card_exp_month . ' / ' . $card_data->card_exp_year,
  );

  $form['card_holder'] = array(
    '#type' => 'item',
    '#title' => t('Card holder'),
    '#markup' => $card_data->card_name,
  );

  $form['card_data'] = array(
    '#type' => 'value',
    '#value' => $card_data,
  );

  return confirm_form(
    $form,
    t('Are you sure you want to delete this card?'),
    'user/' . $card_data->uid . '/cards',
    theme('card_data_overview', array('card_data' => $card_data)),
    t('Delete')
  );
}

/**
 * Form submit handler: delete stored card data.
 */
function commerce_cardonfile_delete_form_submit($form, &$form_state) {
  $card_data = $form_state['values']['card_data'];

  // Invoke the payment method's card delete callback.
  $payment_method = commerce_payment_method_instance_load($card_data->instance_id);
  $callback = $payment_method['cardonfile']['delete callback'];

  if (function_exists($callback)) {
    if (!$callback($form, $form_state, $payment_method, $card_data)) {
      // Display a message if we failed to communicate properly with the payment
      // gateway in the Card on File delete callback.
      drupal_set_message(t('We encountered an error attempting to delete your card data. Please try again and contact us if this error persists.'), 'error');
      $form_state['redirect'] = 'user/' . $card_data->uid . '/cards';
      return;
    }
  }

  // Disable the card data but retain the record.
  $card_data->status = 0;
  commerce_cardonfile_save($card_data);

  drupal_set_message(t('The stored card data has been deleted.'));

  // Redirect to the payment methods tab if the user has other stored payment.
  $stored_cards = commerce_cardonfile_load_multiple_by_uid($card_data->uid);

  if (!empty($stored_cards)) {
    $form_state['redirect'] = 'user/' . $card_data->uid . '/cards';
  }
  else {
    $form_state['redirect'] = 'user/' . $card_data->uid;
  }
}
