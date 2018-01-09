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
    $allowed_units = array_keys(physical_dimension_units());
    $dimensions = self::fromCsv($value);
    $valid = isset($dimensions['width'], $dimensions['length'], $dimensions['height'], $dimensions['unit'])
      && is_numeric($dimensions['width']) && is_numeric($dimensions['length']) && is_numeric($dimensions['height'])
      && in_array($dimensions['unit'], $allowed_units);

    return array(
      'status' => ($valid) ? COMMERCE_XLS_IMPORT_DATA_SUCCESS : COMMERCE_XLS_IMPORT_DATA_ERROR,
      'message' => (!$valid) ? t('Invalid Dimensions (format must look like length=23,height=10,width=20,unit=in)') : NULL,
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
          $key = strtolower(trim($split[0]));
          $value = strtolower(trim($split[1]));
          $unserialized[$key] = $value;
        }
      }
    }

    return $unserialized;
  }

}
