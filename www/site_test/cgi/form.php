<html>
    <head>
        <link rel="stylesheet" href="../site.css" />
    </head>
    <body>
        <form action="/cgi/hello.php" method = "post">
            <label for="fname">First name:</label><br>
            <input type="text" placeholder="<?php if (isset($_COOKIE['fname'])) echo $_COOKIE['fname'] ?>" id="fname" name="fname"><br>
            <label for="lname">Last name:</label><br>
            <input type="text" id="lname" placeholder="<?php if (isset($_COOKIE['lname'])) echo $_COOKIE['lname'] ?>" name="lname"><br><br>
            <input type="submit" value="Submit">
          </form>
    </body>
</html>
