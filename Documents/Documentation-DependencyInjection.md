Dependency Injector
===================

When you build larger applications, their business logic and other systems
need to live somewhere. Stashing everything in helper classes that get used by
the user interface classes works for a while, but once systems need lifetimes
beyond a single dialog, things get messy quickly.

Dependency injection is a technique that lets you cleanly manage such systems
as application-internal "services" while mostly eliminating the tedious work
of managing and wiring up these services.


Basic Example
-------------

Let's assume your application allows users to log in and that this logged-in
state persists even when different dialogs are opened and closed:

```cpp
class LoginService {

  public: void Login(const std::u8string &userName) { /* ... */ }

  public: std::optional<std::u8string> GetLoggedInUser() const { /* ... */ }

  public: void Logout() { /* ... */ }

};
```

Your main window needs to both know whether a user is logged in as well as
the logged-in user's name. Clearly, it needs a reference to your
`LoginService` to do so:

```cpp
class MainWindow {

  public: MainWindow(const std::shared_ptr<LoginService> &loginService) :
    loginService(loginService) { /* ... */ }

  public: void LoginButtonClicked() { /* ... */ }

  private: std::u8string userNameForLoginText;
  private: std::shared_ptr<LoginService> loginService;

};
```

Ordinarily, you would create the login service, stash it somewhere and pass it
to the main window when you create it.

With `Nuclex::Support::Services`, you can instead set it up as a dependency:

```cpp
void bindServices(Nuclex::Support::Services::ServiceCollection &services) {
  // Singleton here means the instance will exist only once inside the service
  // container. It is not the "singleton instance" anti-pattern.
  services.AddSingleton<LoginService>();

  // Note: constructor parameters are detected, adding or removing services
  // from the MainWindow constructor does not require code changes here.
  services.AddTransient<MainWindow>();
}

int main() {
  using Nuclex::Support::Services::StandardServiceCollection;
  using Nuclex::Support::Services::ServiceProvider;

  // Like most modern dependency injectors, the service bindings are set up
  // during initialization and then become immutable in the service provider.
  StandardServiceCollection services;
  bindServices(services);

  // Then the service provider is constructed which owns the instances
  std::shared_ptr<ServiceProvider> sp = services.BuildServiceProvider();

  std::shared_ptr<MainWindow> mainWindow = sp->GetService<MainWindow>();
  runMainLoop(mainWindow);
}
```

What's going on here? We configure and construct a `ServiceProvider`. That's
a kind of container for all application-wide instances of services. They can
be requested from the service provider and are created lazily as needed.

At the bottom of the `main()` method, it requests a `MainWindow` instance.
Here, the `ServiceProvider` will figure out that the `MainWindow` needs
the `LoginService` and construct an instance of the `LoginService` as well,
handing it to the `MainWindow`.

You can already see how this neatly lets you write services and consume them
in other services, dialog classes, view models and so on without producing
spaghetti code - if you establish the convention that services have to be
named `XyService`, developers quickly see that if an `XyService` is consumed
somewhere, it must be an application-internal service found e.g. within
some `Services` directory in your code base.


Service Interfaces
------------------

The above example still produced tightly coupled code. What if you wanted to
test your `MainWindow` with a mock of the service? Besides testability, there
are many other reasons to work with interfaces in larger C++ projects,
including that interfaces can hide implementation details, reduce header
dependencies and avoid rebuilds when their implementation is changed.

So let us refine the example from before:

```cpp
class LoginService {

  public: virtual ~LoginService() = default;

  public: virtual void Login(const std::u8string &userName) = 0;

  public: virtual std::optional<std::u8string> GetLoggedInUser() const = 0;

  public: virtual void Logout() = 0;

};

class WebBasedLoginManager : public LoginService {

  public: WebBasedLoginManager() { /* ... */ }
  public: ~WebBasedLoginManager() = default;

  public: void Login(const std::u8string &userName) override { /* ... */ }

  public: std::optional<std::u8string> GetLoggedInUser() const override {
    /* ... */
  }

  public: void Logout() override { /* ... */ }

};
```

Now the `MainWindow` still depends on the `LoginService`, but
the `LoginService` is just an interface - it lists methods that must be
provided and possibly documents an "interface contract" that states how
these methods must behave.

When you write unit tests, that now allows you to replace the `LoginService`
with a dummy implementation, a so-called "mock" object, through which your
main window's behavior can be verified - does it attempt to log in with
the correct user when `LoginButtonClicked()` is called?

And for actual use, the adjustments in the service bindings are tiny:

```cpp
void bindServices(Nuclex::Support::Services::ServiceCollection &services) {
  services.AddSingleton<LoginService, WebBasedLoginManager>();
  services.AddTransient<MainWindow>();
}

int main() {
  using Nuclex::Support::Services::StandardServiceCollection;
  using Nuclex::Support::Services::ServiceProvider;

  // Like most modern dependency injectors, the service bindings are set up
  // during initialization and then become immutable in the service provider.
  StandardServiceCollection services;
  bindServices(services);

  // Then the service provider is constructed which owns the instances
  std::shared_ptr<ServiceProvider> sp = services.BuildServiceProvider();

  std::shared_ptr<MainWindow> mainWindow = sp->GetService<MainWindow>();
  runMainLoop(mainWindow);
}
```

The only line that changed is the `AddSingleton<>()` call inside of
the `bindServices()` method: the first template argument is the interface by
which other services will depend on the login service and the second template
argument is the actual implementation.

Nifty, right?


Service Scopes
--------------

Up until now, the example has only used two service lifetime types:

- `services.AddSingleton<>()`
- `services.AddTransient<>()`

A singleton service, once requested, has one instance that exists for as long
as the `ServiceProvider` is kept around (hence the name).

A transient service has no managed instances. Each request for a transient
service dishes out a new instance (so each `sp->GetService<MainWindow>()`
produces an entirely new `MainWindow`).

But there's a third service lifetime type: scoped services:

- `services.AddScoped()`

Let's start with another example:

```cpp
class Logger {
  // ...logging stuff...
};
class DatabaseService {
  // ...database access stuff...
};
class HomePageController {

  public: HomePageController(const std::shared_ptr<DatabaseService> &database) :
    database(database) { /* .... */ }
  
  public: WebResponse Index() {
    // ...does some database work...
  }
  
  private: std::shared_ptr<DatabaseService> database;

};
```

This hypothetical web application has a `HomePageController` that, in
a pattern familiar to MVC developers, gets used when a page under an URL slug
such as `https://www.example.com/HomePage/` is requested.
The `HomePageController` is used by a hypothetical `HttpServer` class that
looks at each incoming HTTP request and creates a new
`HomePageController` running in its own thread so separate reqeusts don't get
in each other's way.

Notice that the `HomePageController` depends on the `DatabaseService`?

If you declared the `DatabaseService` to be a singleton service, multiple
`HomePageController` instances could access it at the same time if enough
requests arrived. Then you'd either have to burden your implementation with
"borrowing" and "returning" database connections from a connection pool or
use a mutex to sequentialize database accesses.

This is where scopes are handy. If you instead declared the `DatabaseService`
as a scoped service, each request could get its own instance, bound to
the lifetime of handling the request:

```cpp
void bindServices(Nuclex::Support::Services::ServiceCollection &services) {
  services.AddScoped<Logger, BasicStdOutLogger>();
  services.AddScoped<DatabaseService, LocalSqliteDatabaseStorage>();
  services.AddScoped<HomePageController>();
  services.AddSingleton<HttpServer>();
}

void HttpServer::handleIncomingHttpRequest(const HttpRequest &request) {
  using Nuclex::Support::Services::ServiceScope;
  std::shared_ptr<ServiceScope> scope = this->serviceProvider->CreateScope();

  // Let's leave out any complex routing and controller association code
  // for the sake of keeping this example simple
  try {
    WebResponse response = scope->GetService<HomePageController>()->Index();
    renderResponse(response);
  }
  catch(const std::exception &error) {
    renderErrorPage(error);
  }
}
```

As you can see, each call to `HttpServer::handleIncomingHttpRequest()` would
create its own `ServiceScope`. Such a scope is like a sub-`ServiceProvider`
that creates services registered for a service lifetime type of "scoped."

Scoped services still have access to services in the outer `ServiceProvider`,
but not vice versa: scoped services are only available from a `ServiceScope`
and their instances are dropped again when the `ServiceScope` is destroyed.

Thus, each HTTP request to our example web server would be handled within its
own service scope, meaning that each HTTP request would have its own
`DatabaseService` instance with a separate database connection. Furthermore,
if a controller does not depend on the `DatabaseService`, no database
connection will even be made because services are constructed lazily.

Your only brain teaser is what concept in your application map scopes to.
For a web server, scopes ideally map to incoming requests. For a desktop
application, scopes map very well to dialogs and windows. For a video game,
perhaps they map well to game sessions (anything that happens between
"New Game" and "Exit to Main Menu").


Other Things to Explore
-----------------------

You can also bind services to factory functions, providing your own code to
construct a service.

Another option is the prototype pattern. Instead of specifying the type as
a template argument, you can specify a prototype instance that will be cloned
(by invoking its copy constructor) when the service is activated.
