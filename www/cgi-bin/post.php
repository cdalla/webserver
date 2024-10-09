#! usr/bin/php

<?php
// Automatically parse the POST data
$data = file_get_contents("php://input");
parse_str($data, $post_params);

// Check if the "name" parameter exists
if (isset($post_params['name'])) {
    $user_name = $post_params['name'];

    // Print the user's name to stdout
    echo $user_name;
} else {
    echo "No name provided"; // Print a message if "name" parameter is not set
}
?>
