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
    drupal_set_message(t('Import/Validate Complete'));
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

  $last_import = variable_get('commerce_xls_import_last_import', '');
  if ($last_import != '') {
    $last_import_message = '<h6>' . t('Last Import Completed: @time',
        array(
          '@time' => format_date($last_import['completed'],
            'medium',
            'F j, Y - h:i T'),
        )) . '</h6>';
    $last_import_message .= t('@row/@total products evaluated during last import', array('@row' => $last_import['row'], '@total' => $last_import['total']));
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

  // @TODO - not critical but we could probably move a whole bunch of this formatting stuff into some templates called through theme functions, all this markup screams template
  // https://www.drupal.org/node/715160
  $description = "<h3>" . t('Commerce Import') . "</h2>";
  $description .= "<h5>" . t('Instructions') . "</h5>";
  $description .= "<p>" . t('There are 3 main functions for the Commerce Import, which are:') . "</p>";
  $description .= "<ul>";
  $description .= "<li>" . t('Import Products') . "</li>";
  $description .= "<li>" . t('Validate Products Without Importing') . "</li>";
  $description .= "<li>" . t('Generate Product Templates') . "</li>";
  $description .= "</ul>";
  $description .= "<p>" . t("Using the product templates, a list of products to add may be uploaded using the appropriate file upload section which will then be added to the commerce catalogue. Use the legend below to figure out which fields belong to the base product, or each variation.") . "</p>";
  $description .= "<p>" . t("When an import is complete, a result document will be generated to show how the import went. If the product was imported successfully, the line will be marked green. If the product had an error, the bad fields will be marked red and the product will not be created, nor will it's variations. If a variation has an error, the field will be marked red.") . "</p>";
  $description .= "<p>" . t("If you find an error and wish to change that product in question, make your changes in the error document and submit it using the regular import functionality. This will update all the products in the file and not create new ones.") . "</p>";

  $form['legend']['description'] = array(
    '#markup' => $description,
  );

  $image_instructions = "";
  $image_instructions .= "<h5 id='image-description'>" . t("Image Instructions") . "</h5>";
  $image_instructions .= "<p>" . t('Images may be uploaded through ftp by creating a .zip file that contains the necessary images to the /import_images directory named @strongproduct_images.zip@endstrong. Full path : sites/default/files/import_images/product_images.zip', array('@strong' => '<strong>', '@endstrong' => '</strong>')) . "</p>";

  $form['legend']['images-text'] = array(
    '#markup' => $image_instructions,
  );

  $form['legend']['parent'] = array(
    '#markup' => '<div id="legend-swatches"><div class="excel-swatches swatch-container" ><h6>' . t('Field Colors') . '</h6><div id="parent-swatch" ><div class="swatch"><span>' . t('Parent product') . '</span></div><span>' . t('The fields necessary to create a base product') . '</span></div>',
  );
  $form['legend']['variation'] = array(
    '#markup' => '<div id="variation-swatch"><div class="swatch"><span>' . t('Variation') . '</span></div><span>' . t('The fields that need to be filled out to successfully create a product variation') . '</span></div>',
  );

  $form['legend']['both'] = array(
    '#markup' => '<div id="both-swatch"><div class="swatch">' . t('Parent and variation') . '</span></div><span>' . t('The fields required by both  base product and its variations') . '</span></div></div>',
  );

  $form['legend']['result_colors'] = array(
    '#markup' => '<div class="result-swatches swatch-container" ><h6>' . t('Result Colors') . '</h6>',
  );

  $form['legend']['warning'] = array(
    '#markup' => '<div id="warning-swatch"><div class="swatch"><span>' . t('Warning') . '</span></div><span>' . t("A field highlighted in this color may have something wrong with it, or it hasn't been evaluated") . '</span></div>',
  );
  $form['legend']['error'] = array(
    '#markup' => '<div id="error-swatch"><div  class="swatch"><span>' . t('Error') . '</span></div><span>' . t('A field in this color has had some errors found') . '</span></div>',
  );
  $form['legend']['success'] = array(
    '#markup' => '<div id="success-swatch"><div  class="swatch"><span>' . t('Successfull') . '</span></div><span>' . t('A field in highlighted in this color is correct') . '</span></div></div></div>',
  );

  $form['legend']['field-legend'] = array(
    '#type' => 'fieldset',
    '#title' => t('Field Help'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#prefix' => '<div style="clear:both"></div>',
  );

  $form['legend']['field-legend']['product_title'] = array(
    '#markup' => '<div class="product-field"><h6>' . t('title') . '</h6><span class="product-field-description">' . t('The title of the base product you are creating') . '</span></div><div style="clear:both;"></div>',
  );

  $form['legend']['field-legend']['sku'] = array(
    '#markup' => '<div class="product-field"><h6>' . t('sku') . '</h6><span class="product-field-description">' . t('The sku of the product variation you are creating') . '</span></div><div style="clear:both;"></div>',
  );

  $form['legend']['field-legend']['status'] = array(
    '#markup' => '<div class="product-field"><h6>' . t('status') . '</h6><span class="product-field-description">' . t('The status of the product or variation. 1 if the item is active, 0 if inactive') . '</span></div><div style="clear:both;"></div>',
  );

  $form['legend']['field-legend']['language'] = array(
    '#markup' => '<div class="product-field"><h6>' . t('language') . '</h6><span class="product-field-description">' . t('The language short hand to create the product in. (en, fr etc)') . '</span></div><div style="clear:both;"></div>',
  );

  foreach (commerce_product_types() as $product_type) {
    $fields = commerce_xls_import_get_possible_fields($product_type['type']);
    foreach ($fields as $field) {
      $info = field_info_instance('node', $field, $product_type['type']);

      if (isset($info['description']) && $info['description'] != NULL) {
        // Only display description if it exists.
        $form['legend']['field-legend'][$product_type['type']][$field] = array(
          '#markup' => '<div class="product-field"><h6>' . $field . '</h6><span class="product-field-description">' . t("@description", array('@description' => $info['description'])) . '</span></div><div style="clear:both;"></div>',
        );
      }
    }
  }

  $form['legend']['field-legend']['variation_title'] = array(
    '#markup' => '<div class="product-field"><h6>' . t('Variation Title') . '</h6><span class="product-field-description">' . t('The title of the variation of the product you are creating. If left blank it will use the title of the base product.') . '</span></div><div style="clear:both;"></div>',
  );

  return $form;
}

/**
 * Validation handler for the import start form.
 */
function commerce_xls_import_start_validate($form, &$form_state) {

  if (isset($_FILES['files'])) {
    foreach ($_FILES['files']['tmp_name'] as $field_name => $value) {
      if ($value != NULL && is_uploaded_file($_FILES['files']['tmp_name'][$field_name])) {
        $file = file_save_upload($field_name);
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
      drupal_set_message(t('Import stopping at next parent product'), 'status', FALSE);
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
