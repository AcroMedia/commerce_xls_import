<?php

function commerce_xls_import_get_error($row, $column) {
  $message = '';

  $result = db_query('SELECT message FROM {commerce_xls_import_errors} WHERE row=:row AND column=:column', array(':row' => $row, ':column' => $column))->fetchField();
  if($result) {
    $message = $result;
  }

  return $message;
}

function commerce_xls_import_get_row_error($row) {
  $result = db_query('SELECT message FROM {commerce_xls_import_errors} WHERE row=:row', array(':row' => $row))->fetchField();

  if($result) {
    return TRUE;
  }

  return FALSE;
}

function commerce_xls_import_set_error($row, $column, $message) {
  db_insert('commerce_xls_import_errors')
    ->fields(array(
      'row' => $row,
      'column' => $column,
      'message' => $message,
    ))
    ->execute();
}

/**
 * Gets the columns status.
 *
 * Determines the column status (error, success) for the
 * phpexport export hook to color code warnings and errors.
 *
 * If whole line is good, mark green, else mark white and highlight warnings and
 * errors.
 */
function commerce_xls_import_get_column_status($column, $row) {
  $result = NULL;

  $error = commerce_xls_import_get_error($column, $row);

  if ($error){
    $result = $error;
  }
  else {
    $result = COMMERCE_XLS_IMPORT_DATA_SUCCESS;
  }

  return $result;
}
