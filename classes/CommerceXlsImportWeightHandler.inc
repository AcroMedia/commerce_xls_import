<?php

/**
 * @file
 * Class definition for CommerceXlsImportWeightHandler.
 */

/**
 * Class CommerceXlsImportWeightHandler.
 */
class CommerceXlsImportWeightHandler extends CommerceXlsImportValueHandler implements CommerceXlsImportValueHandlerInterface {

  /**
   * {@inheritdoc}
   */
  public static function validate($value, EntityDrupalWrapper $wrapper, $field_name) {
    $errors = array();
    $allowed_units = array_keys(physical_weight_units());
    $dimensions = self::fromCsv($value);

    if (!isset($dimensions['weight'])) {
      $errors[] = t('The "weight" value is not found.');
    }
    elseif (!is_numeric($dimensions['weight'])) {
      $errors[] = t('The "weight" value must be numeric.');
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
      $weight = self::fromCsv($value);
      module_load_include('module', 'physical', 'physical');

      $field_value = physical_weight_field_data_auto_creation();
      $field_value['weight'] = $weight['weight'];
      $field_value['unit'] = $weight['unit'];

      $wrapper->{$field_name} = $field_value;
    }

    return $valid;
  }

  /**
   * {@inheritdoc}
   */
  public static function get(EntityDrupalWrapper $wrapper, $field_name) {
    $weight = $wrapper->{$field_name}->value();
    return self::toCsv($weight);
  }

  /**
   * {@inheritdoc}
   */
  public static function toCsv($value, $delimiter = ',', $enclosure = '"') {
    $formatted = array();
    if (is_array($value)) {
      foreach ($value as $unit => $measurement) {
        // $unit would be weight/unit.
        // $measurement would be a numerical value or lb/kg etc.
        $unit = strtolower(trim($unit));
        $measurement = strtolower(trim($measurement));
        $formatted[] = ($unit . '=' . $measurement);
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
          // $unit would be weight/unit.
          // $measurement would be a numerical value or lb/kg etc.
          $unit = strtolower(trim($split[0]));
          $measurement = strtolower(trim($split[1]));
          $unserialized[$unit] = $measurement;
        }
      }
    }

    return $unserialized;
  }

}
