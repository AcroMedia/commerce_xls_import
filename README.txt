CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Features
 * Requirements
 * Known Problems

INTRODUCTION
------------

The Commerce XLS Import module is a Drupal Commerce module that allows the
import of Drupal Commerce Products from an XLS file.

 * For a full description of the module, visit the project page:
   https://www.drupal.org/project/commerce_xls_import

 * To submit bug reports and feature suggestions, or to track changes:
   https://www.drupal.org/project/issues/2504005


FEATURES
--------

 * Import Commerce products from XLSX.
 * Import product images.
 * Generate template XLSX for each product type.
 * Exports XLSX results containing information on successful and unsuccessful product
   imports.
 * Export data for all existing products.

REQUIREMENTS
------------

This module was built and tested to work with Commerce Kickstart and may not
work properly with Drupal Commerce depending on your set-up.

 - Modules -

 * Background Process (https://www.drupal.org/project/background_process)
 * Commerce Product (https://www.drupal.org/project/commerce)

 - Libraries -

 * Spout August 2016 dev or higher (https://github.com/box/spout)
   Install in the libraries folder as spout.

KNOWN PROBLEMS
______________

 * Can't set variation titles (https://www.drupal.org/node/2509178)
