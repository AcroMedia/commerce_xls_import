<?php

/**
 * @file
 * Admin page for product exports.
 */

/**
 * Import Menu Form.
 *
 * Contains the buttons export products.
 */
function commerce_xls_import_start() {
  $form = array();
  $types = array();

  foreach (commerce_product_types() as $product_type) {
    $types[$product_type['type']] = $product_type['name'];
  }

  $form['product_type'] = array(
    '#title' => t('Product Type'),
    '#type' => 'select',
    '#description' => 'Select the product type to export.',
    '#options' => $types,
  );

  $form['export'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="product-options ' . t('export') . '">',
    '#suffix' => '</div><div style="clear:both" class="divider"></div>',
  );

  $form['export_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Begin Export'),
  );


  $form['buttons'] = array(
    '#type' => 'markup',
    '#prefix' => '<div style="clear:both"></div><div class="import-buttons">',
    '#suffix' => '</div>',
    '#weight' => 10,
  );

  if (commerce_xls_import_is_running()) {
    drupal_set_message(t('You are currently running an import or validation, you will not be able to run an export until it is finished or stopped manually'), 'warning', FALSE);
  }

  return $form;
}

/**
 * Submit handler for the import start form.
 */
function commerce_xls_import_start_submit($form, &$form_state) {
  $product_type = $form_state['values']['product_type'];
  $product_info = commerce_xls_import_export_product_data($product_type);

  commerce_xls_import_export('export', array($product_info), $product_type);

  $filename = $product_type . "_export.xls";
  $path = file_build_uri($filename);

  readfile($path);

  // Exit is used to keep page from refreshing which would prevent download.
  drupal_exit();
}
