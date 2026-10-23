<?php
session_start();
require_once 'functions.php';
$msg = [];

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $email = trim($_POST['email']);
    $login = trim($_POST['login']);
    $p1 = $_POST['pass1'];
    $p2 = $_POST['pass2'];
    
    if (empty($email) || empty($login) || empty($p1)) $msg[] = "Wypełnij pola.";
    if ($p1 !== $p2) $msg[] = "Hasła się różnią.";
    if (isLoginTaken($login)) $msg[] = "Login zajęty.";

    // obsluga zdjecia profilowego
    if (isset($_FILES['photo']) && $_FILES['photo']['error'] === UPLOAD_ERR_OK) {
        $ext = strtolower(pathinfo($_FILES['photo']['name'], PATHINFO_EXTENSION));
        if (!in_array($ext, ALLOWED_TYPES)) $msg[] = "Zły format pliku.";
        
        if (empty($msg)) {
            $name = "profile_" . md5(time() . $login) . '.' . $ext;
            if (createThumbnail($_FILES['photo']['tmp_name'], PROFILE_DIR . $name, $ext)) {
                registerUser($email, $login, $p1, $name);
                $_SESSION['flash'] = "Konto założone. Zaloguj się.";
                header("Location: login.php"); exit;
            } else {
                $msg[] = "Błąd zapisu zdjęcia.";
            }
        }
    } else {
        $msg[] = "Zdjęcie wymagane.";
    }
}
?>
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <title>Rejestracja</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <?php include 'menu_partial.php'; ?>
    <main class="container">
        <header><h2>Rejestracja</h2></header>
        <?php if (!empty($msg)): ?><div class="message error"><ul><?php foreach ($msg as $e) echo "<li>$e</li>"; ?></ul></div><?php endif; ?>
        <form method="POST" enctype="multipart/form-data">
            <div class="form-group"><label>Email:</label><input type="text" name="email" required></div>
            <div class="form-group"><label>Login:</label><input type="text" name="login" required></div>
            <div class="form-group"><label>Hasło:</label><input type="password" name="pass1" required></div>
            <div class="form-group"><label>Powtórz:</label><input type="password" name="pass2" required></div>
            <div class="form-group"><label>Zdjęcie:</label><input type="file" name="photo" accept="image/*" required></div>
            <button type="submit" class="btn-primary">Załóż konto</button>
        </form>
    </main>
</body>
</html>