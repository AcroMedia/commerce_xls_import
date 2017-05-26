#!/bin/bash

set -e $DRUPAL_TI_DEBUG

# Ensure the right Drupal version is installed.
# The first time this is run, it will install Drupal.
# Note: This function is re-entrant.
drupal_ti_ensure_drupal

# Change to the Drupal directory
cd "$DRUPAL_TI_DRUPAL_DIR"

# Create the the module directory (only necessary for D7)
# For D7, this is sites/default/modules
# For D8, this is modules
mkdir -p "$DRUPAL_TI_DRUPAL_DIR/$DRUPAL_TI_MODULES_PATH"

drush dl commerce
drush dl addressfield rules ctools entity views
drush en -y commerce_cart commerce_customer_ui commerce_product_ui commerce_line_item_ui commerce_order_ui commerce_payment commerce_payment_example commerce_tax_ui simpletest
drush en -y commerce_xls_import
git clone https://github.com/box/spout.git
mv spout sites/all/libraries/spout
drush cc all
wget https://www.drupal.org/files/issues/DATE_RFC7231-2877243-1.patch
git apply DATE_RFC7231-2877243-1.patch
#pecl install zip