<?php
/**
 * @file
 * Template file for the Commerce XLS Import admin list.
 */

$menu_items = commerce_xls_import_menu();

if ($menu_items) {
  $output = '<ul class="xls-action-links">';
  foreach ($menu_items as $key=>$item) {
    if($item['type']!= 132) {
      $output .= '<li class="xls-leaf">' . l($item['title'], $key);
      if (isset($item['description'])) {
        $output .= '<div class="description-xls">' . $item['description'] . '</div>';
      }
      $output .= '</li>';
    }
  }
  $output .= '</ul>';
}
echo $output;
?>
