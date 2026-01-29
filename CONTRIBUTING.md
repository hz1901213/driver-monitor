# Contributing to Windows Driver Monitor

Thank you for your interest in contributing to the Windows Driver Monitor project! This document provides guidelines for contributing.

## Code of Conduct

### Our Pledge
We are committed to providing a welcoming and inclusive environment for all contributors.

### Expected Behavior
- Be respectful and constructive
- Focus on what is best for the project
- Show empathy towards other contributors
- Accept constructive criticism gracefully

## How to Contribute

### Reporting Bugs

Before creating a bug report:
1. Check the [Issues](https://github.com/hz1901213/driver-monitor/issues) page
2. Verify it's not in the [Known Issues](README.md#-known-issues) section
3. Test with the latest version

When creating a bug report, include:
- **Title:** Clear, descriptive summary
- **Environment:**
  - Windows version (e.g., Windows 10 21H2, Windows 11 22H2)
  - Visual Studio version
  - CMake version
  - ImGui version
- **Steps to Reproduce:**
  1. Step-by-step instructions
  2. Include screenshots if applicable
- **Expected Behavior:** What should happen
- **Actual Behavior:** What actually happens
- **Error Messages:** Full error text/screenshots
- **Log Files:** Attach driver_monitor.log if relevant

### Suggesting Features

Feature requests are welcome! Include:
- **Use Case:** Why is this feature needed?
- **Proposed Solution:** How should it work?
- **Alternatives:** Other approaches you've considered
- **Implementation Ideas:** Technical suggestions (optional)

### Submitting Code

#### Development Setup

1. **Fork the repository**
   ```bash
   # Fork on GitHub, then clone your fork
   git clone https://github.com/YOUR_USERNAME/driver-monitor.git
   cd driver-monitor
   ```

2. **Create a branch**
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b fix/your-bug-fix
   ```

3. **Setup development environment**
   ```bash
   # Download ImGui
   git clone https://github.com/ocornut/imgui.git libs/imgui
   
   # Create build directory
   mkdir build
   cd build
   cmake ..
   ```

#### Coding Standards

**C++ Style:**
- Follow existing code style (C++17)
- Use 4 spaces for indentation (no tabs)
- Opening braces on same line for functions
- Clear, descriptive variable names
- Comment complex logic

**Example:**
```cpp
// Good
void DriverMonitor::ProcessDriverEvent(DriverEvent& event) {
    // Set timestamp
    event.timestamp = Utils::GetTimestamp();
    
    // Get signer info if path is available
    if (!event.installPath.empty()) {
        event.signerInfo = Utils::GetSignerInfo(event.installPath);
    }
}

// Avoid
void process(DriverEvent&e){int x=0;/* unclear */}
```

**Naming Conventions:**
- Classes: `PascalCase` (e.g., `EventManager`)
- Functions: `PascalCase` (e.g., `GetTimestamp()`)
- Variables: `camelCase` (e.g., `eventType`)
- Member variables: `m_camelCase` (e.g., `m_eventManager`)
- Constants: `UPPER_CASE` (e.g., `MAX_EVENTS`)

**Error Handling:**
- Check all Windows API return values
- Use RAII for resource management
- Avoid raw `new`/`delete` (use smart pointers)
- Log errors appropriately

**Thread Safety:**
- Document thread access in comments
- Use `std::mutex` for shared data
- Prefer `std::lock_guard` over manual lock/unlock

#### Commit Messages

Format:
```
<type>: <subject>

<body>

<footer>
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, no logic change)
- `refactor`: Code refactoring
- `perf`: Performance improvement
- `test`: Adding/updating tests
- `build`: Build system changes
- `ci`: CI configuration changes

Example:
```
feat: Add export to CSV functionality

- Implemented CSV export in EventLogPanel
- Added menu item in File menu
- Includes all event fields with proper escaping
- Tested with 1000+ events

Closes #42
```

#### Pull Request Process

1. **Update documentation**
   - Add to README.md if user-facing
   - Update ARCHITECTURE.md if design changes
   - Add code comments for complex logic

2. **Test your changes**
   - Build without errors/warnings
   - Test on Windows 10 and/or 11
   - Verify no memory leaks (run for 1+ hour)
   - Check CPU usage is reasonable

3. **Create pull request**
   - Title: Clear summary of changes
   - Description:
     - What changed and why
     - Testing performed
     - Screenshots (for UI changes)
     - Related issues (e.g., "Fixes #123")
   
4. **Code review**
   - Address reviewer feedback
   - Keep discussion constructive
   - Update PR as needed

5. **Merge**
   - Maintainer will merge when approved
   - Branch will be deleted after merge

## Areas for Contribution

### High Priority
- [ ] Full ETW implementation (kernel event tracing)
- [ ] Memory leak fixes
- [ ] Performance optimizations
- [ ] UI/UX improvements
- [ ] Documentation improvements
- [ ] Bug fixes from Issues

### Medium Priority
- [ ] Advanced filtering options
- [ ] Export formats (CSV, JSON, XML)
- [ ] Configuration import/export
- [ ] Keyboard shortcuts
- [ ] Themes/customization
- [ ] Multi-language support

### Low Priority / Future
- [ ] Database backend (SQLite)
- [ ] Kernel driver for blocking
- [ ] Machine learning threat detection
- [ ] Network monitoring integration
- [ ] Web dashboard
- [ ] Automated testing

## Development Guidelines

### Adding a New Monitoring Method

1. Create header/source in `src/monitoring/`
2. Implement `CheckForNewDrivers()` method
3. Add thread in `DriverMonitor.cpp`
4. Update CMakeLists.txt
5. Document in README.md
6. Add configuration options if needed

### Adding a GUI Feature

1. Add UI code in `MainWindow.cpp` (or create new panel)
2. Add state variables in `MainWindow.h`
3. Test with different window sizes
4. Ensure theme colors are consistent
5. Add to README.md usage guide

### Modifying Event Structure

1. Update `DriverEvent` in `Utils.h`
2. Update event creation in monitoring methods
3. Update GUI display in `MainWindow.cpp`
4. Update export/log formats
5. Consider backward compatibility with config

### Performance Considerations

- Profile before optimizing
- Use ImGuiListClipper for large lists
- Minimize mutex lock duration
- Sleep appropriately in monitoring threads
- Test with high event volume (1000+ events)

## Testing

### Manual Testing Checklist

- [ ] Build succeeds without warnings
- [ ] Application starts and shows GUI
- [ ] Start/Stop monitoring works
- [ ] Events appear in real-time
- [ ] All filters work correctly
- [ ] Search functions properly
- [ ] Settings save/load correctly
- [ ] Export creates valid file
- [ ] Whitelist management works
- [ ] No memory leaks (check Task Manager)
- [ ] CPU usage < 15% during monitoring
- [ ] GUI remains responsive

### Test Scenarios

1. **Driver Installation**
   - Install Windows Update with drivers
   - Verify events appear correctly

2. **Filtering**
   - Enable/disable each filter
   - Verify correct events shown/hidden

3. **Long-Running**
   - Run for 1+ hours
   - Check memory usage stable
   - Verify no crashes

4. **Configuration**
   - Change settings
   - Save config
   - Restart application
   - Verify settings persisted

## Documentation

### Code Comments

Use comments for:
- Complex algorithms
- Non-obvious design decisions
- Thread safety notes
- Windows API quirks

Don't comment:
- Obvious code
- Already clear from names

Example:
```cpp
// Good: Explains why
// We must use WARP device if hardware acceleration fails
// because some VMs don't support full DirectX 11
if (res == DXGI_ERROR_UNSUPPORTED) {
    res = D3D11CreateDeviceAndSwapChain(..., D3D_DRIVER_TYPE_WARP, ...);
}

// Bad: States the obvious
// Increment i
i++;
```

### README Updates

Update README.md when:
- Adding user-facing features
- Changing build process
- Fixing significant bugs
- Modifying configuration

## License

By contributing, you agree that your contributions will be licensed under the MIT License. See [LICENSE](LICENSE) file.

## Questions?

- Open an [Issue](https://github.com/hz1901213/driver-monitor/issues) for questions
- Tag with "question" label
- Check existing issues first

## Recognition

Contributors will be:
- Listed in README.md contributors section
- Credited in release notes
- Thanked for their work!

Thank you for contributing to Windows Driver Monitor! 🎉
