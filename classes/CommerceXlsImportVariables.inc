<?php

/**
 * @file
 * CommerceXlsImportVariables class definition.
 */

/**
 * Class containing variables used by Commerce XLS Import.
 */
class CommerceXlsImportVariables {
  private static $validate;
  private static $import;
  private static $status;

  /**
   * Set $status.
   */
  public static function setValidate($value) {
    CommerceXlsImportVariables::$validate = $value;
    CommerceXlsImportVariables::$import = !$value;
    CommerceXlsImportVariables::setStatus();
  }

  /**
   * Set $import.
   */
  public static function setImport($value) {
    CommerceXlsImportVariables::$import = $value;
    CommerceXlsImportVariables::$validate = !value;
    CommerceXlsImportVariables::setStatus();
  }

  /**
   * Set the status message.
   */
  public static function setStatus() {
    if (CommerceXlsImportVariables::$validate) {
      CommerceXlsImportVariables::$status = "Validate";
    }
    elseif (CommerceXlsImportVariables::$import) {
      CommerceXlsImportVariables::$status = "Import";
    }
  }

  /**
   * Get validate.
   */
  public static function isValidate() {
    return CommerceXlsImportVariables::$validate;
  }

  /**
   * Get import.
   */
  public static function isImport() {
    return CommerceXlsImportVariables::$import;
  }

  /**
   * Get the status message.
   */
  public static function getStatus() {
    return CommerceXlsImportVariables::$status;
  }
}
