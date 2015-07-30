<?php

/**
 * @file
 * Admin page and form callbacks for the Commerce XLS Import module.
 */

/**
 * Import Menu Form.
 *
 * Contains the buttons to begin the import or to generate example headers.
 */
function commerce_xls_import_start() {
  $form = array();
  $types = array();

  if (variable_get('commerce_xls_import_completed', '') == 1) {
    drupal_set_message(t('Import/Validate Complete with @errors errors', array('@errors' => variable_get('commerce_xls_import_error_count'))));
    variable_del('commerce_xls_import_completed');
  }

  $form['message'] = array(
    '#markup' => '<div id="import_status_messages" class="messages status" style="display:none;"></div>',
  );

  $form['legend'] = array(
    '#type' => 'fieldset',
    '#title' => t('Instructions'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#weight' => 0,
  );

  $form['legend']['output'] = array(
    '#markup' => theme('commerce_xls_import_instructions'),
  );

  foreach (commerce_product_types() as $product_type) {
    $types[] = $product_type['type'];

    $form[$product_type['type']] = array(
      '#type' => 'markup',
      '#prefix' => '<div class="product-options ' . t('@bundle', array('@bundle' => $product_type['type'])) . '">',
      '#suffix' => '</div><div style="clear:both" class="divider"></div>',
    );

    $form[$product_type['type']]['template-' . $product_type['type']] = array(
      '#type' => 'submit',
      '#value' => t('Download @product_type Template', array('@product_type' => $product_type['type'])),
      '#description' => t('Download a template for @product_type products', array('@product_type' => $product_type['type'])),
      '#attributes' => array('product_type' => $product_type['type']),
    );
    $form[$product_type['type']][$product_type['type']] = array(
      '#type' => 'file',
      '#title' => t('Upload @product_type Import File', array('@product_type' => $product_type['type'])),
      '#description' => t('Upload the .xls file that contains any @product_type products to add to the catalogue', array('@product_type' => $product_type['type'])),
      '#attributes' => array('product_type' => $product_type['type']),
    );
  }

  $form['product_images'] = array(
    '#type' => 'file',
    '#title' => t('Upload Product Images File'),
    '#description' => t('Upload the .zip file that contains any product images to add to the catalogue'),
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
        '@errors' => variable_get('commerce_xls_import_error_count'),
      ));
  }
  else {
    $last_import_message = '';
  }

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

    $form['buttons']['import_button'] = array(
      '#type' => 'submit',
      '#value' => t('Begin Full Import'),
    );
    $form['buttons']['validate_xls'] = array(
      '#type' => 'submit',
      '#value' => t('Validate File'),
    );
    $form['import_status'] = array(
      '#type' => 'hidden',
      '#value' => 0,
    );
  }
  else {
    $form['last-import-message'] = array(
      '#markup' => '<div class="last-import-message"><h6>' . t('Import/Validation in progress') . '</h6></div>',
      '#weight' => 9,
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
 * Validation handler for the import start form.
 */
function commerce_xls_import_start_validate($form, &$form_state) {

  if (isset($_FILES['files'])) {
    foreach ($_FILES['files']['tmp_name'] as $field_name => $value) {
      if ($field_name == 'product_images' && !empty($value)) {
        $regex = '/\.(' . preg_replace('/ +/', '|', preg_quote('zip')) . ')$/i';
        if (!preg_match($regex, $_FILES['files']['name']['product_images'])) {
          $error = t('Product images need to be uploaded in a .zip file');
          drupal_set_message($error, 'error');
        }
        else {
          drupal_move_uploaded_file($value, file_build_uri('product_images.zip'));
        }

      }
      elseif ($value != NULL && is_uploaded_file($_FILES['files']['tmp_name'][$field_name])) {
        $validators = array('file_validate_extensions' => array('xls'));
        $file = file_save_upload($field_name, $validators);
        if ($file) {
          $form_state['files'][] = array('uri' => $file->uri, 'product_type' => $field_name);
        }
      }
    }
  }

  if (!isset($form_state['files']) && ($form_state['clicked_button']['#id'] == 'edit-import-button' || $form_state['clicked_button']['#id'] == 'edit-validate-xls')) {
    foreach ($_FILES['files']['tmp_name'] as $field_name) {
      form_set_error($field_name, t('No files to import'));
    }
  }
}

/**
 * Submit handler for the import start form.
 */
function commerce_xls_import_start_submit($form, &$form_state) {
  global $user;

  if ($form_state['clicked_button']['#id'] == 'edit-import-button') {

    // Reset import tables.
    db_delete('commerce_xls_import')
      ->execute();
    db_delete('commerce_xls_import_errors')
      ->execute();

    foreach ($form_state['files'] as $file) {
      if (!commerce_xls_import_is_running()) {
        $handle = background_process_start('commerce_xls_import_import', $file);

        db_insert('commerce_xls_import')
          ->fields(array(
            'status' => 'pending',
            'handle' => $handle,
            'uid' => $user->uid,
          ))
          ->execute();
      }
      else {
        drupal_set_message(t('You are currently running an import. Please wait until it is finished or cancel the current one'), 'error', FALSE);
      }
    }
  }
  elseif ($form_state['clicked_button']['#id'] == 'edit-validate-xls') {
    $validate = TRUE;

    // Reset import table.
    db_delete('commerce_xls_import')
      ->execute();
    db_delete('commerce_xls_import_errors')
      ->execute();

    foreach ($form_state['files'] as $file) {
      if (!commerce_xls_import_is_running()) {
        $handle = background_process_start('commerce_xls_import_import', $file, $validate);

        db_insert('commerce_xls_import')
          ->fields(array(
            'status' => 'pending',
            'handle' => $handle,
            'uid' => $user->uid,
          ))
          ->execute();
      }
      else {
        drupal_set_message(t('You are currently running an file validation. Please wait until it is finished or cancel the current one'), 'error', FALSE);
      }
    }
  }
  elseif ($form_state['clicked_button']['#id'] == 'edit-zip-button') {
    commerce_xls_import_extract_zip();
  }
  elseif ($form_state['clicked_button']['#id'] == 'edit-import-stop') {
    if (commerce_xls_import_stop_import()) {
      drupal_set_message(t('Import stopping at next product'), 'status', FALSE);
    }
    else {
      drupal_set_message(t('Failed to Stop Import'), 'error', FALSE);
    }
    drupal_goto('admin/commerce/products/import_commerce');
  }
  elseif ($form_state['clicked_button']['#id'] == 'edit-errors-button') {

    $filename = "product_import_results.xls";
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
  else {
    // it's something from an individual product type template download.
    $product_type = $form_state['clicked_button']['#attributes']['product_type'];

    commerce_xls_import_export('template', NULL, $product_type);

    $filename = $product_type . "_template.xls";
    $path = file_build_uri($filename);

    readfile($path);

    // Exit is used to keep page from refreshing which would prevent download.
    drupal_exit();
  }
}
