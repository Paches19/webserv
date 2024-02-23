<?php
session_start();
?>
<!DOCTYPE html>
<html>
<head>
    <title>Document</title>
</head>
<body>
<?php

if ($_SERVER['REQUEST_METHOD'] == 'GET')
{
    $_SESSION["fname"] = $_GET["fname"];
    $_SESSION["lname"] = $_GET["lname"];
    echo("arguments are: ". $_GET["fname"] . " " . $_GET["lname"] . "\n");
}
else if ($_SERVER['REQUEST_METHOD'] == 'POST')
{
    $_SESSION["fname"] = $_POST["fname"];
    $_SESSION["lname"] = $_POST["lname"];
    echo("arguments are: ". $_POST["fname"] . " " . $_POST["lname"] . "\n");
}


?> 
</body>
</html>