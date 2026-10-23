<?php
session_start();
require_once 'functions.php';

// obsluga zapamietywania (koszyk)
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['save_selection'])) {
    if (!isset($_SESSION['saved_photos'])) $_SESSION['saved_photos'] = [];
    
    // sprawdzamy co zaznaczone a co odznaczone
    $visible = isset($_POST['visible_ids']) ? $_POST['visible_ids'] : [];
    $selected = isset($_POST['selected_ids']) ? $_POST['selected_ids'] : [];

    foreach ($visible as $id) {
        if (in_array($id, $selected)) {
            if (!isset($_SESSION['saved_photos'][$id])) $_SESSION['saved_photos'][$id] = 1;
        } else {
            if (isset($_SESSION['saved_photos'][$id])) unset($_SESSION['saved_photos'][$id]);
        }
    }
    header("Location: gallery.php?page=" . ($_GET['page'] ?? 1));
    exit;
}

// pobieranie zdjec
$perPage = 4;
$page = max(1, (int)($_GET['page'] ?? 1));
$userId = isset($_SESSION['user_id']) ? $_SESSION['user_id'] : null;

$total = countImages($userId);
$pages = ceil($total / $perPage);
$images = ($total > 0) ? getGalleryImages($page, $perPage, $userId) : [];
?>
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <title>Galeria</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <?php include 'menu_partial.php'; ?>

    <main class="container wide">
        <header>
            <h2>Galeria Zdjęć</h2>
            <a href="index.php" class="btn-secondary">← Dodaj nowe</a>
            <a href="saved.php" class="btn-secondary" style="background-color: #0056b3; margin-left: 5px;">Zapamiętane →</a>        </header>

        <form method="POST">
            <section class="gallery-grid">
                <?php if (empty($images)): ?>
                    <p style="width:100%; text-align:center;">Brak zdjęć.</p>
                <?php else: ?>
                    <?php foreach ($images as $doc): ?>
                        <?php 
                            $id = (string)$doc['_id']; 
                            $checked = isset($_SESSION['saved_photos'][$id]);
                            $private = isset($doc['privacy']) && $doc['privacy'] === 'private';
                        ?>
                        <div class="gallery-item" style="<?= $checked ? 'border: 2px solid #0056b3; background:#eef;' : '' ?>">
                            <a href="<?= UPLOAD_DIR . $doc['file_name'] ?>" target="_blank">
                                <img src="<?= UPLOAD_DIR . $doc['thumb_name'] ?>" alt="Foto">
                            </a>
                            <div style="padding:10px;">
                                <?php if ($private): ?><span class="badge-private">Prywatne</span><?php endif; ?>
                                <strong><?= htmlspecialchars($doc['title']) ?></strong><br>
                                <small>Autor: <?= htmlspecialchars($doc['author']) ?></small>
                                <hr style="margin: 10px 0; border: 0; border-top: 1px solid #eee;">
                                <label style="cursor:pointer; display:block; text-align:left;">
                                    <input type="checkbox" name="selected_ids[]" value="<?= $id ?>" <?= $checked ? 'checked' : '' ?>>
                                    Wybierz
                                </label>
                                <input type="hidden" name="visible_ids[]" value="<?= $id ?>">
                            </div>
                        </div>
                    <?php endforeach; ?>
                <?php endif; ?>
            </section>

            <div style="text-align:center; margin-top:20px;">
                <button type="submit" name="save_selection" class="btn-primary" style="max-width: 300px;">
                    Zapisz wybór
                </button>
            </div>
        </form>

        <section class="pagination">
            <?php for ($i = 1; $i <= $pages; $i++): ?>
                <a href="?page=<?= $i ?>" class="<?= ($i === $page) ? 'active' : '' ?>"><?= $i ?></a>
            <?php endfor; ?>
        </section>
    </main>
</body>
</html>