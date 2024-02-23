<?php
if ($_SERVER['REQUEST_METHOD'] == 'GET')
{
    setcookie("fname", $_GET['fname'], time()+3600);
    setcookie("lname", $_GET['lname'], time()+3600);
}
else if ($_SERVER['REQUEST_METHOD'] == 'POST')
{
    setcookie("fname", $_POST['fname'], time()+3600);
    setcookie("lname", $_POST['lname'], time()+3600);
}
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
    echo("arguments are: ". $_GET["fname"] . " " . $_GET["lname"] . "\n");
}
else if ($_SERVER['REQUEST_METHOD'] == 'POST')
{
    echo("arguments are: ". $_POST["fname"] . " " . $_POST["lname"] . "\n");
}

?> 
</body>
</html>
