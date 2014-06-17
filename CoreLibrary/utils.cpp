//	utils.cpp
//
//	Author: Eric Nivel, Thor List
//
//	BSD license:
//	Copyright (c) 2010, Eric Nivel, Thor List
//	All rights reserved.
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided that the following conditions are met:
//
//   - Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   - Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//   - Neither the name of Eric Nivel or Thor List nor the
//     names of their contributors may be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
//	THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
//	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//	DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
//	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "utils.h"

#if defined WINDOWS
#include <intrin.h>
#pragma intrinsic (_InterlockedDecrement)
#pragma intrinsic (_InterlockedIncrement)
#pragma intrinsic (_InterlockedExchange)
#pragma intrinsic (_InterlockedExchange64)
#pragma intrinsic (_InterlockedCompareExchange)
#pragma intrinsic (_InterlockedCompareExchange64)
#elif defined LINUX
#ifdef DEBUG
#include <map>
#include <execinfo.h>
#endif // DEBUG
#endif //LINUX

#include <algorithm>
#include <cctype>
#include <chrono>

#define R250_IA (sizeof(uint64)*103)
#define R250_IB (sizeof(uint64)*R250_LEN-R250_IA)
#define R521_IA (sizeof(uint64)*168)
#define R521_IB (sizeof(uint64)*R521_LEN-R521_IA)

namespace core {


#if defined LINUX
bool CalcTimeout(struct timespec &timeout, uint64 ms) {

    struct timeval now;
    if (gettimeofday(&now, NULL) != 0)
        return false;

    timeout.tv_sec = now.tv_sec + ms / 1000;
    long us = now.tv_usec + ms % 1000;
    if (us >= 1000000) {
        timeout.tv_sec++;
        us -= 1000000;
    }
    timeout.tv_nsec = us * 1000; // usec -> nsec
    return true;
}

uint64 GetTime() {
    struct timeval tv;
    if (gettimeofday(&tv, NULL))
        return 0;
    return (tv.tv_usec + tv.tv_sec * 1000000LL);
}
#endif

void Error::PrintBinary(void* p, uint64 size, bool asInt, const char* title) {
    if (title != NULL)
        printf("--- %s %lu ---\n", title, size);
    unsigned char c;
    for (uint64 n = 0; n < size; n++) {
        c = *(((unsigned char*)p) + n);
        if (asInt)
            printf("[%u] ", (unsigned int)c);
        else
            printf("[%c] ", c);
        if ((n > 0) && ((n + 1) % 10 == 0))
            printf("\n");
    }
    printf("\n");
}

SharedLibrary *SharedLibrary::New(const char *fileName) {

    SharedLibrary *sl = new SharedLibrary();
    if (sl->load(fileName))
        return sl;
    else {

        delete sl;
        return NULL;
    }
}

SharedLibrary::SharedLibrary(): library(NULL) {
}

SharedLibrary::~SharedLibrary() {
#if defined WINDOWS
    if (library)
        FreeLibrary(library);
#elif defined LINUX
    if (library)
        dlclose(library);
#endif
}

SharedLibrary *SharedLibrary::load(const char *fileName) {
#if defined WINDOWS
    library = LoadLibrary(TEXT(fileName));
    if (!library) {

        DWORD error = GetLastError();
        std::cerr << "> Error: unable to load shared library " << fileName << " :" << error << std::endl;
        return NULL;
    }
#elif defined LINUX
    /*
    * libraries on Linux are called 'lib<name>.so'
    * if the passed in fileName does not have those
    * components add them in.
    */
    char *libraryName = (char *)calloc(1, strlen(fileName) + 6 + 1);
    /*if (strstr(fileName, "lib") == NULL) {
    strcat(libraryName, "lib");
    }*/
    strcat(libraryName, fileName);
    if (strstr(fileName + (strlen(fileName) - 3), ".so") == NULL) {
        strcat(libraryName, ".so");
    }
    library = dlopen(libraryName, RTLD_NOW | RTLD_GLOBAL);
    if (!library) {
        std::cout << "> Error: unable to load shared library " << fileName << " :" << dlerror() << std::endl;
        free(libraryName);
        return NULL;
    }
    free(libraryName);
#endif
    return this;
}

////////////////////////////////////////////////////////////////////////////////////////////////

void TimeProbe::set() {

    cpu_counts = getCounts();
}

int64 TimeProbe::getCounts() {
#if defined WINDOWS
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
#elif defined LINUX
    static struct timeval tv;
    static struct timezone tz;
    gettimeofday(&tv, &tz);
    return (((int64)tv.tv_sec) * 1000000) + (int64)tv.tv_usec;
#endif
}

void TimeProbe::check() {

    cpu_counts = getCounts() - cpu_counts;
}

uint64 TimeProbe::us() {

    return (uint64)(cpu_counts * Time::Period);
}

////////////////////////////////////////////////////////////////////////////////////////////////

#if defined WINDOWS
typedef LONG NTSTATUS;
typedef NTSTATUS(__stdcall *NSTR)(ULONG, BOOLEAN, PULONG);
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
bool NtSetTimerResolution(IN ULONG RequestedResolution, IN BOOLEAN Set, OUT PULONG ActualResolution);
#elif defined LINUX
// TODO
#endif

float64 Time::Period;

int64 Time::InitTime;

void Time::Init() {
    InitTime = Get();
    Period = 1;
    //Period = (std::chrono::steady_clock::period.num / 1000.0) / std::chrono::steady_clock::period.den;
/*    Period = (float64)std::chrono::high_resolution_clock::period::num
            / (float64)std::chrono::high_resolution_clock::period::den;
    Period *= 1000;
    debug("time") << "time resolution:" << Period << std::chrono::high_resolution_clock::period::num << std::chrono::high_resolution_clock::period::den;*/
}

uint64 Time::Get() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

std::string Time::ToString_seconds(uint64 t) {

    uint64 us = t % 1000;
    uint64 ms = t / 1000;
    uint64 s = ms / 1000;
    ms = ms % 1000;

    std::string _s = String::Uint2String(s);
    _s += "s:";
    _s += String::Uint2String(ms);
    _s += "ms:";
    _s += String::Uint2String(us);
    _s += "us";

    return _s;
}

std::string Time::ToString_year(uint64 t) {

    uint64 us = t % 1000;
    uint64 ms = t / 1000;
//uint64 s=ms/1000;
    ms = ms % 1000;

    time_t _gmt_time;
    time(&_gmt_time);
    struct tm *_t = gmtime(&_gmt_time);

    std::string _s = asctime(_t); // _s is: Www Mmm dd hh:mm:ss yyyy but we want: Www Mmm dd yyyy hh:mm:ss:msmsms:ususus
    std::string year = _s.substr(_s.length() - 5, 4);
    _s.erase(_s.length() - 6, 5);
    std::string hh_mm_ss = _s.substr(_s.length() - 9, 8);
    hh_mm_ss += ":";
    hh_mm_ss += String::Uint2String(ms);
    hh_mm_ss += ":";
    hh_mm_ss += String::Uint2String(us);

    _s.erase(_s.length() - 9, 9);
    _s += year;
    _s += " ";
    _s += hh_mm_ss;
    _s += " GMT";

    return _s;
}

////////////////////////////////////////////////////////////////////////////////////////////////

uint8 Host::Name(char *name) {
#if defined WINDOWS
    DWORD s = 255;
    GetComputerName(name, &s);
    return (uint8)s;
#elif defined LINUX
    struct utsname utsname;
    uname(&utsname);
    strcpy(name, utsname.nodename);
    return strlen(name);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////

#if defined WINDOWS
const uint64 Semaphore::Infinite = INFINITE;
#elif defined LINUX
/*
 * Normally this should be SEM_VALUE_MAX but apparently the <semaphore.h> header
 * does not define it. The documents I have read indicate that on Linux it is
 * always equal to INT_MAX - so use that instead.
 */
const uint64 Semaphore::Infinite = INT_MAX;
#endif

Semaphore::Semaphore(uint64 initialCount, uint64 maxCount) {
#if defined WINDOWS
    s = CreateSemaphore(NULL, initialCount, maxCount, NULL);
#elif defined LINUX
    sem_init(&s, 0, initialCount);
#endif
}

Semaphore::~Semaphore() {
#if defined WINDOWS
    CloseHandle(s);
#elif defined LINUX
    sem_destroy(&s);
#endif
}

bool Semaphore::acquire(uint64 timeout) {
#if defined WINDOWS
    uint64 r = WaitForSingleObject(s, timeout);
    return r == WAIT_TIMEOUT;
#elif defined LINUX
    struct timespec t;
    int r;

    CalcTimeout(t, timeout);
    r = sem_timedwait(&s, &t);
    return r == 0;
#endif
}

void Semaphore::release(uint64 count) {
#if defined WINDOWS
    ReleaseSemaphore(s, count, NULL);
#elif defined LINUX
    for (uint64 c = 0; c < count; c++)
        sem_post(&s);
#endif
}

void Semaphore::reset() {
#if defined WINDOWS
    bool r;
    do
        r = acquire(0);
    while (!r);
#elif defined LINUX
    bool r;
    do
        r = acquire(0);
    while (!r);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////

#if defined WINDOWS
const uint64 Timer::Infinite = INFINITE;
#elif defined LINUX
const uint64 Timer::Infinite = INT_MAX;

static void timer_signal_handler(int sig, siginfo_t *siginfo, void *context) {
    SemaTex* sematex = (SemaTex*) siginfo->si_value.sival_ptr;
    if (sematex == NULL)
        return;
    pthread_mutex_lock(&sematex->mutex);
    pthread_cond_broadcast(&sematex->semaphore);
    pthread_mutex_unlock(&sematex->mutex);
}
#endif

Timer::Timer() {
#if defined WINDOWS
    t = CreateWaitableTimer(NULL, false, NULL);
    if (t == NULL) {
        printf("Error creating timer\n");
    }
#elif defined LINUX
    pthread_cond_init(&sematex.semaphore, NULL);
    pthread_mutex_init(&sematex.mutex, NULL);

    struct sigaction sa;
    struct sigevent timer_event;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO; /* Real-Time signal */
    sa.sa_sigaction = timer_signal_handler;
    sigaction(SIGRTMIN, &sa, NULL);

    memset(&timer_event, 0, sizeof(sigevent));
    timer_event.sigev_notify = SIGEV_SIGNAL;
    timer_event.sigev_signo = SIGRTMIN;
    timer_event.sigev_value.sival_ptr = (void *)&sematex;
    int ret = timer_create(CLOCK_REALTIME, &timer_event, &timer);
    if (ret != 0) {
        printf("Error creating timer: %d\n", ret);
    }
#endif
}

Timer::~Timer() {
#if defined WINDOWS
    CloseHandle(t);
#elif defined LINUX
    pthread_cond_destroy(&sematex.semaphore);
    pthread_mutex_destroy(&sematex.mutex);
    timer_delete(timer);
#endif
}

void Timer::start(uint64 deadline, uint64 period) {
#if defined WINDOWS
    LARGE_INTEGER _deadline; // in 100 ns intervals
    _deadline.QuadPart = -10LL * deadline; // negative means relative
    bool r = SetWaitableTimer(t, &_deadline, (long)period, NULL, NULL, 0);
    if (!r) {
        printf("Error arming timer\n");
    }
#elif defined LINUX
    struct itimerspec newtv;
    sigset_t allsigs;

    uint64 t = deadline;
    uint64 p = period * 1000;
    newtv.it_interval.tv_sec = p / 1000000;
    newtv.it_interval.tv_nsec = (p % 1000000) * 1000;
    newtv.it_value.tv_sec = t / 1000000;
    newtv.it_value.tv_nsec = (t % 1000000) * 1000;

    pthread_mutex_lock(&sematex.mutex);

    int ret = timer_settime(timer, 0, &newtv, NULL);
    if (ret != 0) {
        printf("Error arming timer: %d\n", ret);
    }
    sigemptyset(&allsigs);

    pthread_mutex_unlock(&sematex.mutex);
#endif
}

bool Timer::wait(uint64 timeout) {
#if defined WINDOWS
    uint64 r = WaitForSingleObject(t, timeout);
    return r == WAIT_TIMEOUT;
#elif defined LINUX
    bool res;
    struct timespec ttimeout;

    pthread_mutex_lock(&sematex.mutex);
    if (timeout == INT_MAX) {
        res = (pthread_cond_wait(&sematex.semaphore, &sematex.mutex) == 0);
    }
    else {
        CalcTimeout(ttimeout, timeout);
        res = (pthread_cond_timedwait(&sematex.semaphore, &sematex.mutex, &ttimeout) == 0);
    }
    pthread_mutex_unlock(&sematex.mutex);
    return res;
#endif
}

bool Timer::wait(uint64 &us, uint64 timeout) {

    TimeProbe probe;
    probe.set();
    bool r = wait((uint64)timeout);
    probe.check();
    us = probe.us();
    return r;
}

////////////////////////////////////////////////////////////////////////////////////////////////

Event::Event() {
#if defined WINDOWS
    e = CreateEvent(NULL, true, false, NULL);
#elif defined LINUX
// TODO.
#endif
}

Event::~Event() {
#if defined WINDOWS
    CloseHandle(e);
#elif defined LINUX
// TODO.
#endif
}

void Event::wait() {
#if defined WINDOWS
    WaitForSingleObject(e, INFINITE);
#elif defined LINUX
// TODO.
#endif
}

void Event::fire() {
#if defined WINDOWS
    SetEvent(e);
#elif defined LINUX
// TODO.
#endif
}

void Event::reset() {
#if defined WINDOWS
    ResetEvent(e);
#elif defined LINUX
// TODO.
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////

void SignalHandler::Add(signal_handler h) {
#if defined WINDOWS
    if (SetConsoleCtrlHandler(h, true) == 0) {

        int e = GetLastError();
        std::cerr << "Error: " << e << " failed to add signal handler" << std::endl;
        return;
    }
#elif defined LINUX
    signal(SIGABRT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGBUS, SIG_IGN);
// signal(SIGHUP, h);
    signal(SIGTERM, h);
    signal(SIGINT, h);
    signal(SIGABRT, h);
// signal(SIGFPE, h);
// signal(SIGILL, h);
// signal(SIGSEGV, h);
#endif
}

void SignalHandler::Remove(signal_handler h) {
#if defined WINDOWS
    SetConsoleCtrlHandler(h, false);
#elif defined LINUX
    signal(SIGABRT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGBUS, SIG_IGN);
// signal(SIGHUP, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
// signal(SIGFPE, SIG_DFL);
// signal(SIGILL, SIG_DFL);
// signal(SIGSEGV, SIG_DFL);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////

uint8 BSR(word data) {
#if defined WINDOWS
#if defined ARCH_32
    DWORD index;
    _BitScanReverse(&index, data);
    return (uint8)index;
#elif defined ARCH_64
    uint64 index;
    _BitScanReverse64(&index, data);
    return (uint8)index;
#endif
#elif defined LINUX
#if defined ARCH_32
    return (uint8)(31 - __builtin_clz((uint64_t)data));
#elif defined ARCH_64
    return (uint8)(63 - __builtin_clzll((uint64_t)data));
#endif
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////

FastSemaphore::FastSemaphore(uint64 initialCount, uint64 maxCount): Semaphore(initialCount > 0 ? 1 : 0, 1), count(initialCount), maxCount(maxCount) {
}

FastSemaphore::~FastSemaphore() {
}

void FastSemaphore::acquire()
{
    int64 c;
    while ((c = --count) >= maxCount); // release calls can bring count over maxCount: acquire has to exhaust these extras
    if (c < 0)
        Semaphore::acquire();
}

void FastSemaphore::release()
{
    int64 c = ++count;
    if (c <= 0)
        Semaphore::release();
}

////////////////////////////////////////////////////////////////////////////////////////////////
/*
 FastMutex::FastMutex(uint64 initialCount):Semaphore(initialCount,1),count(initialCount){
 }

 FastMutex::~FastMutex(){
 }

 void FastMutex::acquire(){

 int64 former=Atomic::Swap32(&count,0);
 if(former==0)
 Semaphore::acquire();
 }

 void FastMutex::release(){

 int64 former=Atomic::Swap32(&count,1);
 if(former==0)
 Semaphore::release();
 }
 */
////////////////////////////////////////////////////////////////////////////////////////////////

bool Error::PrintLastOSErrorMessage(const char* title) {
    int64 err = Error::GetLastOSErrorNumber();
    char buf[1024];
    if (!Error::GetOSErrorMessage(buf, 1024, err))
        printf("%s: [%lu] (could not get error message)\n", title, err);
    else
        printf("%s: [%lu] %s\n", title, err, buf);
    return true;
}

int64 Error::GetLastOSErrorNumber() {
#ifdef WINDOWS
    int64 err = WSAGetLastError();
    WSASetLastError(0);
    return err;
#else
    return (int64) errno;
#endif
}

bool Error::GetOSErrorMessage(char* buffer, uint64 buflen, int64 err) {
    if (buffer == NULL)
        return false;
    if (buflen < 512) {
        strcpy(buffer, "String buffer not large enough");
        return false;
    }
    if (err < 0)
        err = Error::GetLastOSErrorNumber();

#ifdef WINDOWS
    if (err == WSANOTINITIALISED) {
        strcpy(buffer, "Cannot initialize WinSock!");
    }
    else if (err == WSAENETDOWN) {
        strcpy(buffer, "The network subsystem or the associated service provider has failed");
    }
    else if (err == WSAEAFNOSUPPORT) {
        strcpy(buffer, "The specified address family is not supported");
    }
    else if (err == WSAEINPROGRESS) {
        strcpy(buffer, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function");
    }
    else if (err == WSAEMFILE) {
        strcpy(buffer, "No more socket descriptors are available");
    }
    else if (err == WSAENOBUFS) {
        strcpy(buffer, "No buffer space is available. The socket cannot be created");
    }
    else if (err == WSAEPROTONOSUPPORT) {
        strcpy(buffer, "The specified protocol is not supported");
    }
    else if (err == WSAEPROTOTYPE) {
        strcpy(buffer, "The specified protocol is the wrong type for this socket");
    }
    else if (err == WSAESOCKTNOSUPPORT) {
        strcpy(buffer, "The specified socket type is not supported in this address family");
    }
    else if (err == WSAEADDRINUSE) {
        strcpy(buffer, "The socket's local address is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs during execution of the bind function, but could be delayed until this function if the bind was to a partially wildcard address (involving ADDR_ANY) and if a specific address needs to be committed at the time of this function");
    }
    else if (err == WSAEINVAL) {
        strcpy(buffer, "The socket has not been bound with bind");
    }
    else if (err == WSAEISCONN) {
        strcpy(buffer, "The socket is already connected");
    }
    else if (err == WSAENOTSOCK) {
        strcpy(buffer, "The descriptor is not a socket");
    }
    else if (err == WSAEOPNOTSUPP) {
        strcpy(buffer, "The referenced socket is not of a type that supports the listen operation");
    }
    else if (err == WSAEADDRNOTAVAIL) {
        strcpy(buffer, "The specified address is not a valid address for this machine");
    }
    else if (err == WSAEFAULT) {
        strcpy(buffer, "The name or namelen parameter is not a valid part of the user address space, the namelen parameter is too small, the name parameter contains an incorrect address format for the associated address family, or the first two bytes of the memory block specified by name does not match the address family associated with the socket descriptor s");
    }
    else if (err == WSAEMFILE) {
        strcpy(buffer, "The queue is nonempty upon entry to accept and there are no descriptors available");
    }
    else if (err == SOCKETWOULDBLOCK) {
        strcpy(buffer, "The socket is marked as nonblocking and no connections are present to be accepted");
    }
    else if (err == WSAETIMEDOUT) {
        strcpy(buffer, "Attempt to connect timed out without establishing a connection");
    }
    else if (err == WSAENETUNREACH) {
        strcpy(buffer, "The network cannot be reached from this host at this time");
    }
    else if (err == WSAEISCONN) {
        strcpy(buffer, "The socket is already connected (connection-oriented sockets only)");
    }
    else if (err == WSAECONNREFUSED) {
        strcpy(buffer, "The attempt to connect was forcefully rejected");
    }
    else if (err == WSAEAFNOSUPPORT) {
        strcpy(buffer, "Addresses in the specified family cannot be used with this socket");
    }
    else if (err == WSAEADDRNOTAVAIL) {
        strcpy(buffer, "The remote address is not a valid address (such as ADDR_ANY)");
    }
    else if (err == WSAEALREADY) {
        strcpy(buffer, "A nonblocking connect call is in progress on the specified socket");
    }
    else if (err == WSAECONNRESET) {
        strcpy(buffer, "Connection was reset");
    }
    else if (err == WSAECONNABORTED) {
        strcpy(buffer, "Software caused connection abort");
    }
    else {
        strcpy(buffer, "Socket error with no description");
    }

#else
    strcpy(buffer, strerror(err));
#endif

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////

bool WaitForSocketReadability(socket s, int64 timeout) {

    int maxfd = 0;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    fd_set rdds;
// create a list of sockets to check for activity
    FD_ZERO(&rdds);
// specify mySocket
    FD_SET(s, &rdds);

#ifdef WINDOWS
#else
    maxfd = s + 1;
#endif

    if (timeout > 0) {
        ldiv_t d = ldiv(timeout * 1000, 1000000);
        tv.tv_sec = d.quot;
        tv.tv_usec = d.rem;
    }

// Check for readability
    int ret = select(maxfd, &rdds, NULL, NULL, &tv);
    return (ret > 0);
}

bool WaitForSocketWriteability(socket s, int64 timeout) {

    int maxfd = 0;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    fd_set wds;
// create a list of sockets to check for activity
    FD_ZERO(&wds);
// specify mySocket
    FD_SET(s, &wds);

#ifdef WINDOWS
#else
    maxfd = s + 1;
#endif

    if (timeout > 0) {
        ldiv_t d = ldiv(timeout * 1000, 1000000);
        tv.tv_sec = d.quot;
        tv.tv_usec = d.rem;
    }

// Check for readability
    return (select(maxfd, NULL, &wds, NULL, &tv) > 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////

bool String::StartsWith(const std::string &s, const std::string &str) {
    if (str.size() > s.size()) return false;
    return (s.compare(0, str.length(), str) == 0);
}

bool String::EndsWith(const std::string &s, const std::string &str) {
    if (str.size() > s.size()) return false;
    return (s.compare(s.size() - str.size(), str.size(), str) == 0);
}

void String::MakeUpper(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(), toupper);
}

void String::MakeLower(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
}

void String::Trim(std::string& str, const char* chars2remove)
{
    TrimLeft(str, chars2remove);
    TrimRight(str, chars2remove);
}

void String::TrimLeft(std::string& str, const char* chars2remove)
{
    if (!str.empty())
    {
        std::string::size_type pos = str.find_first_not_of(chars2remove);

        if (pos != std::string::npos)
            str.erase(0, pos);
        else
            str.erase(str.begin() , str.end()); // make empty
    }
}

void String::TrimRight(std::string& str, const char* chars2remove)
{
    if (!str.empty())
    {
        std::string::size_type pos = str.find_last_not_of(chars2remove);

        if (pos != std::string::npos)
            str.erase(pos + 1);
        else
            str.erase(str.begin() , str.end()); // make empty
    }
}


void String::ReplaceLeading(std::string& str, const char* chars2replace, char c)
{
    if (!str.empty())
    {
        std::string::size_type pos = str.find_first_not_of(chars2replace);

        if (pos != std::string::npos)
            str.replace(0, pos, pos, c);
        else
        {
            int n = str.size();
            str.replace(str.begin(), str.end() - 1, n - 1, c);
        }
    }
}

std::string String::Int2String(int64 i) {
    char buffer[1024];
    sprintf(buffer, "%ld", i);
    return std::string(buffer);
}

std::string String::Uint2String(uint64 i) {
    char buffer[1024];
    sprintf(buffer, "%lu", i);
    return std::string(buffer);
}
}
