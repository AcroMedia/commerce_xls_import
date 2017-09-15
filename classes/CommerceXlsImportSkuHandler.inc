<?php
/**
 * @file
 * Class definition for CommerceXlsImportSkuHandler.
 */

/**
 * Class CommerceXlsImportSkuHandler
 */
class CommerceXlsImportSkuHandler extends CommerceXlsImportValueHandler implements CommerceXlsImportValueHandlerInterface {

  /**
   * Generates a SKU for the product.
   *
   * @param $wrapper
   *   The product object.
   * @param $sku
   *   A string containing a SKU.
   */
  public static function generate_sku($wrapper) {
    $product_info = commerce_product_types();
    $product_type = $wrapper->type->value();
    $pattern = $product_info[$product_type]['autosku']['pattern'];
    $case = $product_info[$product_type]['autosku']['advanced']['case'];
    $sku = commerce_autosku_generate_sku($wrapper->value(), $pattern, $case);

    return $sku;
  }

  /**
   * {@inheritdoc}
   */
  public static function validate($value, \EntityDrupalWrapper $wrapper, $field_name) {
    $valid = commerce_product_validate_sku($value) && commerce_product_validate_sku_unique($value, $wrapper->product_id->value());

    return array(
      'status' => ($valid) ? COMMERCE_XLS_IMPORT_DATA_SUCCESS : COMMERCE_XLS_IMPORT_DATA_ERROR,
      'message' => (!$valid) ? t('Invalid SKU.') : NULL,
    );
  }

  /**
   * {@inheritdoc}
   */
  public static function set($value, \EntityDrupalWrapper $wrapper, $field_name) {
    if(empty($value)) {
      $value = self::generate_sku($wrapper);
    }

    $valid = self::validate($value, $wrapper, $field_name);

    if ($valid['status'] === COMMERCE_XLS_IMPORT_DATA_SUCCESS) {
      $wrapper->{$field_name} = $value;
    }

    return $valid;
  }

  /**
   * {@inheritdoc}
   */
  public static function get(\EntityDrupalWrapper $wrapper, $field_name) {
    return $wrapper->{$field_name}->value();
  }

}
