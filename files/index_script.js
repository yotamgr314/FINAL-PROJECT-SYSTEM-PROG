const form = document.getElementById('userForm');
const loginBtn = document.getElementById('loginBtn');
const registerBtn = document.getElementById('registerBtn');
const messageSpan = document.getElementById('responseMessage');

// Get the "response" parameter from the URL
const response = new URLSearchParams(window.location.search).get("response");

if (response) {
    messageSpan.style.display = 'inline';
    messageSpan.classList.remove('text-danger', 'text-success'); // Reset previous styles

    if (response === 'RegisterFailedUserAlreadyExists') {
        messageSpan.textContent = 'Registration failed: Username already exists!';
        messageSpan.classList.add('text-danger');
    } else if (response === 'dbErrorFailedToAppendNewData') {
        messageSpan.textContent = 'Error: Could not save data. Try again later.';
        messageSpan.classList.add('text-danger');
    } else if (response === 'RegisterSuccess') {
        messageSpan.textContent = 'Registration successful!';
        messageSpan.classList.add('text-success');
    } else if (response === 'LoginFailed') {
        messageSpan.textContent = 'Login failed: Invalid username or password.';
        messageSpan.classList.add('text-danger');
    }
}

// Handle form submission
form.addEventListener('submit', function(event) {
    if (document.activeElement === loginBtn) {
        form.action = '/login';
    } else if (document.activeElement === registerBtn) {
        form.action = '/register';
    }
});
