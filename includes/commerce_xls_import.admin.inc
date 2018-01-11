<?php

/**
 * @file
 * Admin page and form callbacks for the Commerce XLS Import module.
 */

/**
 * Import Menu Form.
 *
 * Contains the buttons to begin the import or to generate example headers.
 *
 * @param array $form
 *   The form that is being built.
 * @param array $form_state
 *   The form state array, passed by reference.
 *
 * @return
 *  The form array representing the from to initiate the import or generate examples headers.
 */
function commerce_xls_import_start($form, &$form_state) {
  $form = array();
  $product_types = array();
  $display_types = array();

  $form['title'] = array(
    '#markup' => '<h1>' . t('Commerce XLS Import') . '</h1>',
  );

  $form['legend'] = array(
    '#type' => 'fieldset',
    '#title' => t('Instructions'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  );

  $form['legend']['output'] = array(
    '#markup' => theme('commerce_xls_import_instructions'),
  );

  foreach (commerce_product_types() as $product_type) {
    $product_types[$product_type['type']] = $product_type['name'];
  }
  if (empty($product_types)) {
    $product_types['no_product_types'] = t('No product types were found');
  }

  $form['product_type'] = array(
    '#title' => t('Product Type'),
    '#type' => 'select',
    '#description' => t('Select the product type to use in import.'),
    '#options' => $product_types,
    '#attributes' => array(
      'id' => array('commerce-xls-import-product-type'),
    ),
  );

  // Only grab node types that have a reference to a product.
  $node_types = commerce_xls_import_get_node_types_with_product_reference();
  if (empty($node_types)) {
    $display_types['no_display_types'] = t('No product display types were found that have a product reference field');
  }
  else {
    foreach ($node_types as $node_type) {
      $display_types[$node_type->type] = $node_type->name;
    }
  }

  $form['display_type'] = array(
    '#title' => t('Display Type'),
    '#type' => 'select',
    '#description' => t('Select the display (node) type to use in import. Only node types with product references are listed.'),
    '#options' => $display_types,
    '#ajax' => array(
      'callback' => 'commerce_xls_import_ajax_display_type_callback',
      'wrapper' => 'commerce-xls-import-product-ref-field',
    ),
  );

  $node_type = isset($form_state['values']['display_type']) ? $form_state['values']['display_type'] : reset($node_types)->type;
  $form['product_ref_field'] = commerce_xls_import_start_product_ref_field($node_type);

  $form['download_template'] = array(
      '#type' => 'submit',
      '#name' => 'download_template',
      '#value' => t('Download Template'),
      '#suffix' => '<div style="clear:both"></div>',
      '#attributes' => array(
          'id' => array('commerce-xls-import-download-template'),
      ),
  );

  $form['import_file'] = array(
    '#type' => 'file',
    '#title' => t('Import File'),
    '#description' => t('Upload the .xlsx file that contains products of the selected type'),
    '#attributes' => array(
      'id' => array('commerce-xls-import-file-upload'),
    ),
  );

  $form['product_images'] = array(
    '#type' => 'file',
    '#title' => t('Upload Product Images File'),
    '#description' => t('Upload the .zip file that contains any product images to add to the catalogue'),
    '#attributes' => array(
      'id' => array('commerce-xls-import-image-upload'),
    ),
  );

  $last_import = variable_get('commerce_xls_import_last_import', '');
  if ($last_import != '') {
    $last_import_message = '<h6>' . t('Last Import Completed: @time',
        array(
          '@time' => format_date($last_import['completed'],
            'medium',
            'F j, Y - h:i T'),
        )) . '</h6>';
    $last_import_message .= t('@row/@total products evaluated during last import with @errors errors',
      array(
        '@row' => $last_import['row'],
        '@total' => $last_import['total'],
        '@errors' => $last_import['error_count'],
      ));
  }
  else {
    $last_import_message = '';
  }

  // Create a container for other modules to add custom settings to.
  $form['settings'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
  );

  $form['buttons'] = array(
    '#type' => 'markup',
    '#prefix' => '<div style="clear:both"></div><div class="import-buttons">',
    '#suffix' => '</div>',
    '#weight' => 10,
  );

  if (!commerce_xls_import_is_running()) {
    $form['last-import-message'] = array(
      '#markup' => '<div class="last-import-message">' . $last_import_message . '</div>',
      '#weight' => 9,
    );
    $form['buttons']['validate'] = array(
      '#type' => 'submit',
      '#value' => t('Validate File'),
    );
    $form['buttons']['import'] = array(
        '#type' => 'submit',
        '#value' => t('Begin Full Import'),
    );
    $form['import_status'] = array(
      '#type' => 'hidden',
      '#value' => 0,
    );
  }
  else {
    $form['last-import-message']['label'] = array(
      '#markup' => '<div class="last-import-message"><h6>' . t('Import/Validation in progress') . '</h6></div>',
    );
    $form['last-import-message']['progress'] = array(
      '#markup' => '<div id="import_status_messages" class="" style="display:none;"></div>',
    );
    $form['buttons']['import_stop'] = array(
      '#type' => 'submit',
      '#value' => t('Stop Import/Validation'),
    );

    $form['import_status'] = array(
      '#type' => 'hidden',
      '#value' => 1,
    );
    drupal_set_message(t('You are currently running an import or validation, you will not be able to run another until it is finished or stopped manually'), 'warning', FALSE);
  }

  $form['buttons']['errors_button'] = array(
    '#type' => 'submit',
    '#value' => t('Retrieve Latest Results'),
  );

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
function commerce_xls_import_start_product_ref_field($bundle) {
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
    '#description' => t('Select the product reference field to use for the product variations.'),
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
function commerce_xls_import_ajax_display_type_callback($form) {
  return $form['product_ref_field'];
}

/**
 * Validation handler for the import start form.
 *
 * @param array $form
 *   The form that is being validated.
 * @param array $form_state
 *   The form state array, passed by reference.
 */
function commerce_xls_import_start_validate($form, &$form_state) {
  // Check that the product type exists.
  $product_type = $form_state['values']['product_type'];
  if ($product_type == 'no_product_types') {
    form_set_error('product_type', t('No product types were found'));
  }
  else {
    if (!commerce_product_type_get_name($product_type)) {
      form_set_error('product_type', t('The product type %type was not found', array('%type' => $product_type)));
    }
  }

  // Check that the display type exists and has a product reference.
  $display_type = $form_state['values']['display_type'];
  if ($display_type == 'no_display_types') {
    form_set_error('display_type', t('No product display types were found that had a product reference'));
  }
  else {
    if (!commerce_xls_import_product_reference_field_exists($display_type)) {
      form_set_error('display_type', t('The product display type %type was not found with a product reference field', array('%type' => $display_type)));
    }
  }

  // Check that the product reference field exists on the product display.
  $product_ref_field = $form_state['values']['product_ref_field'];
  $field = field_info_instance('node', $product_ref_field, $display_type);
  if (!$field) {
    form_set_error('product_ref_field', t('The product reference field %type does not exist on the product display %node', array('%type' => $product_ref_field, '%node' => $display_type)));
  }

  if (isset($_FILES['files'])) {
    foreach ($_FILES['files']['tmp_name'] as $field_name => $tmp_file) {
      if ($field_name == 'product_images' && !empty($tmp_file)) {
        if (pathinfo($_FILES['files']['name'][$field_name], PATHINFO_EXTENSION) !== 'zip') {
          $error = t('Product images need to be uploaded in a .zip file');
          drupal_set_message($error, 'error');
        }
        else {
          drupal_move_uploaded_file($tmp_file, CommerceXlsImportSettings::imageZipUri());
        }

      }
      elseif ($tmp_file != NULL && is_uploaded_file($_FILES['files']['tmp_name'][$field_name])) {
        if (pathinfo($_FILES['files']['name'][$field_name], PATHINFO_EXTENSION) !== 'xlsx') {
          form_set_error('import_file', t('Only XLSX files are allowed.'));
        }
        else {
          $file_name = "temporary://{$_FILES['files']['name'][$field_name]}";
          drupal_move_uploaded_file($tmp_file, $file_name);
          $form_state['files'][] = array('uri' => $file_name, 'product_type' => $field_name);
        }
      }
    }
  }

  if (!isset($form_state['files']) && ($form_state['clicked_button']['#id'] == 'edit-import' || $form_state['clicked_button']['#id'] == 'edit-validate')) {
    foreach ($_FILES['files']['tmp_name'] as $field_name) {
      form_set_error($field_name, t('No files to import'));
    }
  }
}

/**
 * Submit handler for the import start form.
 *
 * @param $form
 *  The form that is being submitted.
 *
 * @param $form_state
 *  The form state array, passed by reference.
 */
function commerce_xls_import_start_submit($form, &$form_state) {
  global $user;

  // Default settings to an empty array as it will not exist unless another
  // module adds custom settings.
  $settings = (!empty($form_state['values']['settings'])) ? $form_state['values']['settings'] : array();
  CommerceXlsImportSettings::setSettings($settings);

  if ($form_state['triggering_element']['#id'] == 'edit-import') {
    // Reset import tables.
    db_delete('commerce_xls_import')
      ->execute();

    foreach ($form_state['files'] as $file) {
      $file['product_type'] = $form_state['values']['product_type'];
      $file['display_type'] = $form_state['values']['display_type'];
      $file['product_ref_field'] = $form_state['values']['product_ref_field'];
      if (!commerce_xls_import_is_running()) {

        db_insert('commerce_xls_import')
          ->fields(array(
            'status' => 'pending',
            'handle' => '',
            'uid' => $user->uid,
          ))
          ->execute();

        commerce_xls_import_main($file, FALSE, $settings);
      }
      else {
        drupal_set_message(t('You are currently running an import. Please wait until it is finished or cancel the current one'), 'error', FALSE);
      }
    }
  }
  elseif ($form_state['triggering_element']['#id'] == 'edit-validate') {
    // Reset import table.
    db_delete('commerce_xls_import')
      ->execute();

    foreach ($form_state['files'] as $file) {
      $file['product_type'] = $form_state['values']['product_type'];
      $file['display_type'] = $form_state['values']['display_type'];
      $file['product_ref_field'] = $form_state['values']['product_ref_field'];
      if (!commerce_xls_import_is_running()) {

        db_insert('commerce_xls_import')
          ->fields(array(
            'status' => 'pending',
            'handle' => '',
            'uid' => $user->uid,
          ))
          ->execute();

        commerce_xls_import_main($file, TRUE, $settings);
      }
      else {
        drupal_set_message(t('You are currently running an file validation. Please wait until it is finished or cancel the current one'), 'error', FALSE);
      }
    }
  }
  elseif ($form_state['triggering_element']['#id'] == 'edit-import-stop') {
    if (commerce_xls_import_stop_import()) {
      drupal_set_message(t('Import stopping at next product'), 'status', FALSE);
    }
    else {
      drupal_set_message(t('Failed to Stop Import'), 'error', FALSE);
    }
    drupal_goto('admin/commerce/products/commerce_xls_import');
  }
  elseif ($form_state['clicked_button']['#id'] == 'edit-errors-button') {

    $filename = 'product_import_results.xlsx';
    $path = file_build_uri($filename);

    if (is_readable($path)) {
      header('Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet');
      header('Content-Disposition: attachment;filename="' . $filename . '"');
      header('Cache-Control: max-age=0');
      readfile($path);

      // Exit is used to keep page from refreshing which would prevent download.
      drupal_exit();
    }
    else {
      drupal_set_message(t('No results found'), 'error');
    }
  }
  elseif (isset($form_state['values']['download_template']) && $form_state['values']['download_template'] == $form_state['triggering_element']['#value']) {
    $product_type = $form_state['values']['product_type'];
    $display_type = $form_state['values']['display_type'];

    commerce_xls_import_template($product_type, $display_type);

    $filename = $product_type . '_template.xlsx';
    $path = file_build_uri($filename);

    readfile($path);

    // Exit is used to keep page from refreshing which would prevent download.
    drupal_exit();
  }
}

/**
 * Form callback for the alias configuration page.
 */
function commerce_xls_import_aliases_form($form, &$form_state) {
  $default_values = variable_get('commerce_xls_import_field_aliases', array());

  $form['heading'] = array(
    '#markup' => '<h1>' . t('Field Aliases') . '</h1>',
  );

  $form['help'] = array(
    '#markup' => '<p>' . t('These settings determine what will be used in the header of the import and export spreadsheets. The default value is the machine name of the field. Note that all fields and properties for all bundles of the commerce_product and node entities are present here but may not be relevant to your import.') . '</p>',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['aliases'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
  );

  $fields = commerce_xls_import_get_possible_fields();
  $fields = $fields['commerce_product'] + $fields['node'];
  $fields = array_keys($fields);

  // Order fields alphabetically by label to make it easier to find.
  asort($fields);

  foreach ($fields as $field_name) {
    $form['aliases'][$field_name] = array(
      '#type' => 'textfield',
      '#title' => $field_name,
      '#default_value' => (!empty($default_values[$field_name])) ? $default_values[$field_name] : $field_name,
    );
  }

  $form['submit_bottom'] = $form['submit'];

  return $form;
}

/**
 * Submit callback for the alias configuration page.
 */
function commerce_xls_import_aliases_form_submit($form, &$form_state) {
  variable_set('commerce_xls_import_field_aliases', $form_state['values']['aliases']);
  drupal_set_message(t('Settings saved.'));
}
