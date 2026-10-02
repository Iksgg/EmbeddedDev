*** Settings ***
Library    String
Library    helper.py

*** Variables ***
${com}     /dev/ttyACM0
${baud}    115200
${board}      nRF5340

*** Test Cases ***
Connect Serial
    Log To Console    Connecting to ${board} on ${com}
    Open Serial Port    ${com}  ${baud}

Test Correct Time Strings
    ${response}=    Send And Read    000005\n
    Log To Console    Received: ${response}
    Should Contain    ${response}    Timer set for 5
    
    ${response}=    Send And Read    000120\n
    Log To Console    Received: ${response}
    Should Contain    ${response}    Timer set for 80

Test Invalid Bounds And Values
    ${response}=    Send And Read    240000\n
    Log To Console    Received: ${response}
    Should Contain    ${response}    Incorrect time

Test Invalid Lengths
    ${response}=    Send And Read    12345\n
    Log To Console    Received: ${response}
    Should Contain    ${response}    Incorrect time

Test Non-Numeric Characters
    ${response}=    Send And Read    12345A\n
    Log To Console    Received: ${response}
    Should Contain    ${response}    Incorrect time

Disconnect Serial
    Log To Console    Disconnecting ${board}
    [TearDown]    close_serial_port