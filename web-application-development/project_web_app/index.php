<?php
session_start();
require_once 'functions.php';

define('MAX_FILE_SIZE', 1 * 1024 * 1024); 
$messages = [];

// sprawdzamy czy formularz wyslany
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $title = isset($_POST['title']) ? trim($_POST['title']) : '';
    
    // autor z sesji lub wpisany recznie
    if (isset($_SESSION['user_id'])) {
        $author = $_SESSION['user_login'];
    } else {
        $author = isset($_POST['author']) ? trim($_POST['author']) : '';
    }

    // prywatnosc
    $privacy = isset($_POST['privacy']) ? $_POST['privacy'] : 'public';
    if (!isset($_SESSION['user_id'])) $privacy = 'public';

    $errors = [];
    if (strlen($title) < 3) $errors[] = "Tytuł za krótki.";
    if (strlen($author) < 3) $errors[] = "Autor za krótki.";

    // sprawdzamy plik
    if (isset($_FILES['file']) && $_FILES['file']['error'] === UPLOAD_ERR_OK) {
        $tmp = $_FILES['file']['tmp_name'];
        $name = $_FILES['file']['name'];
        $size = $_FILES['file']['size'];
        $ext = strtolower(pathinfo($name, PATHINFO_EXTENSION));

        if (!in_array($ext, ALLOWED_TYPES)) $errors[] = "Tylko JPG/PNG.";
        if ($size > MAX_FILE_SIZE) $errors[] = "Plik max 1MB.";

        if (empty($errors)) {
            $baseName = md5(time() . $name);
            $newName = $baseName . '.' . $ext;
            $thumbName = "thumb_" . $baseName . '.' . $ext;

            // przenoszenie i miniatura
            if(move_uploaded_file($tmp, UPLOAD_DIR . $newName)) {
                if (createThumbnail(UPLOAD_DIR . $newName, UPLOAD_DIR . $thumbName, $ext)) {
                    try {
                        // zapis do bazy
                        $userId = isset($_SESSION['user_id']) ? $_SESSION['user_id'] : null;
                        saveImageToDB($newName, $thumbName, $title, $author, $userId, $privacy);
                        $_SESSION['msg'] = ['type' => 'success', 'text' => "Zdjęcie dodane!"];
                    } catch (Exception $e) {
                        $_SESSION['msg'] = ['type' => 'error', 'text' => "Błąd bazy."];
                    }
                } else {
                    $_SESSION['msg'] = ['type' => 'success', 'text' => "Zapisano, ale błąd miniatury."];
                }
            } else {
                $_SESSION['msg'] = ['type' => 'error', 'text' => "Błąd zapisu pliku."];
            }
        } else {
            $_SESSION['msg'] = ['type' => 'error', 'text' => implode('<br>', $errors)];
        }
    } elseif (!empty($errors)) {
        $_SESSION['msg'] = ['type' => 'error', 'text' => implode('<br>', $errors)];
    }

    // przeladowanie zeby wyczyscic post
    header("Location: index.php");
    exit;
}

$msg = [];
if (isset($_SESSION['msg'])) {
    $msg = $_SESSION['msg'];
    unset($_SESSION['msg']);
}
?>
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <title>Upload</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <?php include 'menu_partial.php'; ?>

    <main class="upload-container">
        <header>
            <h2>Dodaj zdjęcie</h2>
            <a href="gallery.php" class="btn-secondary">Galeria →</a>
        </header>

        <?php if (!empty($msg)): ?>
            <div class="message <?= $msg['type'] ?>"><?= $msg['text'] ?></div>
        <?php endif; ?>

        <form method="POST" enctype="multipart/form-data">
            <div class="form-group">
                <label>Tytuł:</label>
                <input type="text" name="title" required>
            </div>

            <div class="form-group">
                <label>Autor:</label>
                <?php if (isset($_SESSION['user_id'])): ?>
                    <input type="text" name="author" value="<?= htmlspecialchars($_SESSION['user_login']) ?>" readonly style="background-color: #e9ecef;">
                    <small style="color: green;">✓ Uzupełniono automatycznie</small>
                <?php else: ?>
                    <input type="text" name="author" required placeholder="Twoje imię">
                <?php endif; ?>
            </div>

            <?php if (isset($_SESSION['user_id'])): ?>
                <div class="form-group" style="padding: 10px; border: 1px solid #ccc;">
                    <label>Widoczność:</label>
                    <label style="display:inline-block; margin-right: 15px;">
                        <input type="radio" name="privacy" value="public" checked> 🌍 Publiczne
                    </label>
                    <label style="display:inline-block;">
                        <input type="radio" name="privacy" value="private"> 🔒 Prywatne
                    </label>
                </div>
            <?php endif; ?>

            <div class="form-group">
                <label>Plik:</label>
                <input type="file" name="file" accept="image/*" required>
            </div>
            <button type="submit" class="btn-primary">Wyślij</button>
        </form>
    </main>
</body>
</html>