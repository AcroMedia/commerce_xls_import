<?php
/**
 * @file
 * Class definition for CommerceXlsImportTextHandler.
 */

/**
 * Class CommerceXlsImportTextLongHandler
 */
class CommerceXlsImportTextLongHandler extends CommerceXlsImportValueHandler implements CommerceXlsImportValueHandlerInterface {

  /**
   * {@inheritdoc}
   */
  public static function set($value, \EntityDrupalWrapper $wrapper, $field_name) {
    $field_info = field_info_field($field_name);
    $multivalue = ((int) $field_info['cardinality'] !== 1);

    $field_info = $wrapper->{$field_name}->info();
    $field = $wrapper->{$field_name}->value();

    // handle plain text format text_long fields per normal
    if (!isset($field_info['type']) || $field_info['type'] != 'text_formatted') {
      return CommerceXlsImportValueHandler::set($value, $wrapper, $field_name);
    }

    if ($multivalue) {
      $value = array();
      $values = self::fromCsv($value);
      foreach ($values as $key => $val) {
        $format = isset($field[$key]['format']) ? $field[$key]['format'] : 'full_html';
        $value[] = array(
          'value' => $val,
          'format' => $format,
          'safe_value' => check_markup($val, $format),
        );
      }
    }
    else {
      $format = isset($field['format']) ? $field['format'] : 'full_html';
      $value = array(
        'value' => $value,
        'format' => $format,
        'safe_value' => check_markup($value, $format),
      );
    }

    $wrapper->{$field_name} = $value;

    return array(
      'status' => COMMERCE_XLS_IMPORT_DATA_SUCCESS,
    );
  }

  /**
   * {@inheritdoc}
   */
  public static function get(\EntityDrupalWrapper $wrapper, $field_name) {
    $field_info = field_info_field($field_name);
    $multivalue = ((int) $field_info['cardinality'] !== 1);

    $field_info = $wrapper->{$field_name}->info();
    $value = $wrapper->{$field_name}->value();

    // handle plain text format text_long fields per normal
    if (!isset($field_info['type']) || $field_info['type'] != 'text_formatted') {
      return CommerceXlsImportValueHandler::get($wrapper, $field_name);
    }

    if ($multivalue) {
      $value = array_map(function ($item) {
        return $item['value'];
      }, $value);
      $value = self::toCsv($value);
    }
    else {
      $value = $value['value'];
    }

    return $value;
  }
}
