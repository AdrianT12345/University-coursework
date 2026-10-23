<?php
session_start();
require_once 'functions.php';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // usuwanie zaznaczonych
    if (isset($_POST['remove_selected']) && isset($_POST['remove_ids'])) {
        foreach ($_POST['remove_ids'] as $id) {
            if (isset($_SESSION['saved_photos'][$id])) unset($_SESSION['saved_photos'][$id]);
        }
    }
    // zmiana ilosci
    if (isset($_POST['quantities'])) {
        foreach ($_POST['quantities'] as $id => $qty) {
            if ((int)$qty > 0 && isset($_SESSION['saved_photos'][$id])) {
                $_SESSION['saved_photos'][$id] = (int)$qty;
            }
        }
    }
}

// pobieramy tylko te co w sesji
$ids = isset($_SESSION['saved_photos']) ? array_keys($_SESSION['saved_photos']) : [];
$images = !empty($ids) ? getImagesByIds($ids) : [];
?>
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <title>Zapamiętane</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <?php include 'menu_partial.php'; ?>

    <main class="container wide">
        <header>
            <h2>Zapamiętane zdjęcia</h2>
        </header>

        <form method="POST">
            <?php if (empty($images)): ?>
                <p style="text-align:center;">Brak zdjęć w schowku.</p>
            <?php else: ?>
                <table style="width:100%; border-collapse: collapse; margin-bottom: 20px;">
                    <thead>
                        <tr style="border-bottom: 2px solid #ccc; text-align:left;">
                            <th style="padding:10px;">X</th>
                            <th style="padding:10px;">Obraz</th>
                            <th style="padding:10px;">Opis</th>
                            <th style="padding:10px;">Ilość</th>
                        </tr>
                    </thead>
                    <tbody>
                        <?php foreach ($images as $doc): ?>
                            <?php 
                                $id = (string)$doc['_id'];
                                $qty = $_SESSION['saved_photos'][$id] ?? 1;
                            ?>
                            <tr style="border-bottom: 1px solid #eee;">
                                <td style="padding:10px;"><input type="checkbox" name="remove_ids[]" value="<?= $id ?>"></td>
                                <td style="padding:10px;"><img src="<?= UPLOAD_DIR . $doc['thumb_name'] ?>" style="width:80px;"></td>
                                <td style="padding:10px;">
                                    <strong><?= htmlspecialchars($doc['title']) ?></strong><br>
                                    <small><?= htmlspecialchars($doc['author']) ?></small>
                                </td>
                                <td style="padding:10px;">
                                    <input type="number" name="quantities[<?= $id ?>]" value="<?= $qty ?>" min="1" style="width: 60px;">
                                </td>
                            </tr>
                        <?php endforeach; ?>
                    </tbody>
                </table>
                <div style="display:flex; justify-content:space-between;">
                    <button type="submit" name="remove_selected" class="btn-secondary" style="background:#c00; color:white;">Usuń zaznaczone</button>
                    <button type="submit" name="update_cart" class="btn-primary" style="width:auto;">Zapisz ilości</button>
                </div>
            <?php endif; ?>
        </form>
    </main>
</body>
</html>