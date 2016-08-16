<?php

/**
 * @file
 * CommerceXlsImportSettings class definition.
 */

/**
 * Class CommerceXlsImportSettings contains settings used by Commerce XLS Import.
 */
class CommerceXlsImportSettings {
  private static $type = 'Import';
  private static $rowErrorStatus = array();

  /**
   * sets the import status to validate
   */
  public static function setValidate() {
    self::setType('Validate');
  }

  /**
   * @param $type string of the status to set
   */
  private static function setType($type) {
    self::$type = $type;
  }

  /**
   * @return bool true if the status is validate
   */
  public static function isValidate() {
    return self::isType('Validate');
  }

  /**
   * @return bool true if the status is import
   */
  public static function isImport() {
    return self::isType('Import');
  }

  /**
   * @param $type string the status you wish to check
   * @return bool if the status matches
   */
  private static function isType($type) {
    return $type == self::$type;
  }

  /**
   * @param $row int row to set the error for
   * @param $status
   */
  public static function setRowError($row) {
    self::$rowErrorStatus[$row] = TRUE;
  }

  /**
   * @param $row int row to test against
   * @return bool return true of row has errors
   */
  public static function rowHasError($row) {
    if (isset(self::$rowErrorStatus[$row])) {
      return TRUE;
    }

    return FALSE;
  }

}
