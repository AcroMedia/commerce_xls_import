<?php

/**
 * @file
 * Admin page for product exports.
 */

/**
 * Import Menu Form.
 *
 * Contains the buttons export products.
 *
 * @param array $form
 *   The form that is being built.
 * @param array $form_state
 *   The form state array, passed by reference.
 *
 * @return array
 *   The array containing the export and import buttons.
 */
function commerce_xls_import_start_export($form, &$form_state) {
  $form = array();
  $product_types = array();
  $display_types = array();

  foreach (commerce_product_types() as $product_type) {
    $product_types[$product_type['type']] = $product_type['name'];
  }
  if (empty($product_types)) {
    $product_types['no_product_types'] = t('No product types were found');
  }

  // Only grab node types that have a reference to a product.
  $node_types = array_filter(node_type_get_types(), function ($node_type) {
    return commerce_xls_import_product_reference_field_exists($node_type->type);
  });
  if (empty($node_types)) {
    $display_types['no_display_types'] = t('No product display types were found that have a product reference field');
  }
  else {
    foreach ($node_types as $node_type) {
      $display_types[$node_type->type] = $node_type->name;
    }
  }

  $export_uri = file_build_uri('export');
  $files = file_scan_directory($export_uri, '/.*/');
  if (!empty($files)) {
    $form['downloads'] = array(
      '#type' => 'fieldset',
      '#title' => t('Download Exports'),
    );

    foreach ($files as $file) {
      $url = file_create_url($file->uri);
      $real_path = drupal_realpath($file->uri);
      $modified = date('Y-m-d H:i:s', filemtime($real_path));

      $form['downloads'][] = array(
        '#markup' => l($file->name . ' (' . $modified . ')', $url),
      );
    }
  }

  $form['title'] = array(
    '#markup' => '<h1>' . t('Commerce XLS Import - Export') . '</h1>',
  );

  $form['product_type'] = array(
    '#title' => t('Product Type'),
    '#type' => 'select',
    '#description' => t('Select the product type to export.'),
    '#options' => $product_types,
  );

  $form['display_type'] = array(
    '#title' => t('Display Type'),
    '#type' => 'select',
    '#description' => t('Select the display (node) type to export. (The node must have a product reference field to show up in this list.)'),
    '#options' => $display_types,
    '#ajax' => array(
      'callback' => 'commerce_xls_import_export_ajax_display_type_callback',
      'wrapper' => 'commerce-xls-import-product-ref-field',
    ),
  );

  $node_type = isset($form_state['values']['display_type']) ? $form_state['values']['display_type'] : reset($node_types)->type;
  $form['product_ref_field'] = commerce_xls_import_start_export_product_ref_field($node_type);

  // Create a container for other modules to add custom settings to.
  $form['settings'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
  );

  $form['export'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="product-options ' . t('export') . '">',
    '#suffix' => '</div><div style="clear:both"></div>',
  );

  $library = libraries_load('spout');
  if ($library['loaded'] === FALSE) {
    drupal_set_message(t('The Spout library could not be loaded. It must be installed to export products.'), 'error');
  }

  if (!commerce_xls_import_is_running()) {
    $form['export_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Begin Export'),
      '#disabled' => ($library['loaded'] === FALSE) ? TRUE : FALSE,
    );
  }
  else {
    $form['warning'] = array(
      '#markup' => '<h6 style="color: red">' . t('Please wait for the current import/validation to complete') . '</h6>',
    );
    drupal_set_message(t('You are currently running an import or validation, you will not be able to run an export until it is finished or stopped manually'), 'warning', FALSE);
  }

  return $form;
}

/**
 * Generates the "Product Reference Field" select list.
 *
 * @param string $bundle
 *   The node type to search for product references fields.
 *
 * @return array
 *   The form array for the product reference select field.
 */
function commerce_xls_import_start_export_product_ref_field($bundle) {
  $product_ref_types = array();
  foreach (field_info_instances('node', $bundle) as $field_name => $instance) {
    $field = field_info_field($field_name);
    if ($field['type'] == 'commerce_product_reference') {
      $product_ref_types[$field_name] = $field_name;
    }
  }

  if (empty($product_ref_types)) {
    return NULL;
  }

  return array(
    '#title' => t('Product Reference Field'),
    '#type' => 'select',
    '#description' => t('Select the product reference field to use in export.'),
    '#prefix' => '<div id="commerce-xls-import-product-ref-field">',
    '#suffix' => '</div>',
    '#options' => $product_ref_types,
  );
}

/**
 * Ajax callback for "Display Type" select field.
 *
 * @param array $form
 *   The form that is being submitted.
 *
 * @return array
 *   Returns the form element for the product reference type.
 */
function commerce_xls_import_export_ajax_display_type_callback($form) {
  return $form['product_ref_field'];
}

/**
 * Submit handler for the import start form.
 *
 * @param array $form
 *   The form containing the values being submitted.
 * @param array $form_state
 *   The form state array, passed by reference.
 */
function commerce_xls_import_start_export_submit(array $form, array &$form_state) {
  if (commerce_xls_import_is_running()) {
    drupal_set_message(t('You are currently running an file validation. Please wait until it is finished or cancel the current one'), 'error', FALSE);
    return;
  }

  $product_type = $form_state['values']['product_type'];
  $display_type = $form_state['values']['display_type'];
  $product_ref_field = $form_state['values']['product_ref_field'];

  // Default settings to an empty array as it will not exist unless another
  // module adds custom settings.
  $settings = (!empty($form_state['values']['settings'])) ? $form_state['values']['settings'] : array();

  commerce_xls_import_export_process($product_type, $display_type, $product_ref_field, $settings);
}
