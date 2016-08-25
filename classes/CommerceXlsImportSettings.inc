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
   * Field aliases keyed by field or property name.
   *
   * @var array
   */
  private static $aliases = array();

  /**
   * Field or property names keyed by their alias.
   *
   * @var array
   */
  private static $field_names = array();

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

  /**
   * Set the list of field aliases.
   *
   * @param array $aliases
   *   An array of aliases where keys are field or property names and values are
   *   the alias for that field.
   */
  public static function setAliases($aliases) {
    self::$aliases = $aliases;
    self::$field_names = array_flip($aliases);
  }

  /**
   * Get the field or property name for an alias.
   *
   * @param string $alias
   *   A field name that may be an alias.
   *
   * @return string
   */
  public static function field_name($alias) {
    return (!empty(self::$field_names[$alias])) ? self::$field_names[$alias] : $alias;
  }

  /**
   * Get the alias for a field or property name.
   *
   * @param string $field_name
   *   A field name.
   *
   * @return string
   */
  public static function field_alias($field_name) {
    return (!empty(self::$aliases[$field_name])) ? self::$aliases[$field_name] : $field_name;
  }

}
