function SendStringToDiscord($string, $webhookuri) {
    $payload = @{'content' = $string}
    Invoke-RestMethod -Uri $webhookuri -Method 'post' -Body $payload
}
