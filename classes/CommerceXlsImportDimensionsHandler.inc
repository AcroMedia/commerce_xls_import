<?php

/**
 * @file
 * Class definition for CommerceXlsImportDimensionsHandler.
 */

/**
 * Class CommerceXlsImportDimensionsHandler.
 */
class CommerceXlsImportDimensionsHandler extends CommerceXlsImportValueHandler implements CommerceXlsImportValueHandlerInterface {

  /**
   * {@inheritdoc}
   */
  public static function validate($value, EntityDrupalWrapper $wrapper, $field_name) {
    $errors = array();
    $allowed_units = array_keys(physical_dimension_units());
    $dimensions = self::fromCsv($value);

    foreach (array('width', 'length', 'height') as $unit) {
      if (!isset($dimensions[$unit])) {
        $errors[] = t('The "!unit" value is not found.', array('!unit' => $unit));
      }
      elseif (!is_numeric($dimensions[$unit])) {
        $errors[] = t('The "!unit" value must be numeric.', array('!unit' => $unit));
      }
    }

    if (!isset($dimensions['unit'])) {
      $errors[] = t('The "unit" value is not found.');
    }
    elseif (!in_array($dimensions['unit'], $allowed_units)) {
      $allowed_units_formatted = implode(', ', $allowed_units);
      $errors[] = t('The "unit" value must be one of the following: !allowed_units_formatted.', array('!allowed_units_formatted' => $allowed_units_formatted));
    }

    $valid = empty($errors);
    return array(
      'status' => $valid ? COMMERCE_XLS_IMPORT_DATA_SUCCESS : COMMERCE_XLS_IMPORT_DATA_ERROR,
      'message' => $valid ? NULL : implode(' ', $errors),
    );
  }

  /**
   * {@inheritdoc}
   */
  public static function set($value, EntityDrupalWrapper $wrapper, $field_name) {
    $valid = self::validate($value, $wrapper, $field_name);

    if ($valid['status'] === COMMERCE_XLS_IMPORT_DATA_SUCCESS && !empty($value)) {
      $dimensions = self::fromCsv($value);
      module_load_include('module', 'physical', 'physical');

      $field_value = physical_dimensions_field_data_auto_creation();
      $field_value['length'] = $dimensions['length'];
      $field_value['width'] = $dimensions['width'];
      $field_value['height'] = $dimensions['height'];
      $field_value['unit'] = $dimensions['unit'];

      $wrapper->{$field_name} = $field_value;
    }

    return $valid;
  }

  /**
   * {@inheritdoc}
   */
  public static function get(EntityDrupalWrapper $wrapper, $field_name) {
    $dimensions = $wrapper->{$field_name}->value();
    return self::toCsv($dimensions);
  }

  /**
   * {@inheritdoc}
   */
  public static function toCsv($value, $delimiter = ',', $enclosure = '"') {
    $formatted = array();
    if (is_array($value)) {
      foreach ($value as $k => $v) {
        // $k would be length/width/height/unit.
        // $v would be a numerical value or in/ft etc.
        $k = strtolower(trim($k));
        $v = strtolower(trim($v));
        $formatted[] = ($k . '=' . $v);
      }
    }
    return parent::toCsv($formatted);
  }

  /**
   * {@inheritdoc}
   */
  public static function fromCsv($value) {
    $dimensions = parent::fromCsv($value);
    $unserialized = array();

    if (is_array($dimensions)) {
      foreach ($dimensions as $dimension) {
        $dimension = trim($dimension);
        $split = explode('=', $dimension);
        if (count($split) == 2) {
          // $k would be length/width/height/unit.
          // $v would be a numerical value or in/ft etc.
          $k = strtolower(trim($split[0]));
          $v = strtolower(trim($split[1]));
          $unserialized[$k] = $v;
        }
      }
    }

    return $unserialized;
  }

}
