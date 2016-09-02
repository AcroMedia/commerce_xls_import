<?php
/**
 * @file
 * Class definition for CommerceXlsImportCommercePriceHandler.
 */

/**
 * Class CommerceXlsImportCommercePriceHandler
 */
class CommerceXlsImportCommercePriceHandler extends CommerceXlsImportValueHandler implements CommerceXlsImportValueHandlerInterface {

  /**
   * {@inheritdoc}
   */
  public static function validate($value, $wrapper, $field_name) {
    $valid = is_numeric($value);
    return array(
      'status' => ($valid) ? COMMERCE_XLS_IMPORT_DATA_SUCCESS : COMMERCE_XLS_IMPORT_DATA_ERROR,
      'message' => (!$valid) ? t('Must be a valid number.') : NULL,
    );
  }

  /**
   * {@inheritdoc}
   */
  public static function set($value, $wrapper, $field_name) {
    $valid = self::validate($value, $wrapper, $field_name);

    if ($valid['status'] === COMMERCE_XLS_IMPORT_DATA_SUCCESS) {
      $currency_code = commerce_default_currency();
      $value = commerce_currency_decimal_to_amount($value, $currency_code);
      $wrapper->{$field_name} = array('amount' => $value, 'currency_code' => $currency_code);
    }

    return $valid;
  }

  /**
   * {@inheritdoc}
   */
  public static function get($wrapper, $field_name) {
    $price = $wrapper->{$field_name}->value();
    return (string) commerce_currency_amount_to_decimal($price['amount'], $price['currency_code']);
  }
}
