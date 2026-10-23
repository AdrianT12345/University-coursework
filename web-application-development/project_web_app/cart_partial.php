<?php
// zliczanie elementow
$total = 0;
if (isset($_SESSION['saved_photos']) && is_array($_SESSION['saved_photos'])) {
    foreach ($_SESSION['saved_photos'] as $qty) {
        $total += (int)$qty;
    }
}
?>
<div class="cart-icon">
    <a href="saved.php" style="text-decoration: none; color: inherit;">
        Zapamiętane: <strong><?= $total ?></strong>
    </a>
</div>