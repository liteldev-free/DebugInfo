# DeThunk
This is the header preprocessor.

### About FakeSymbol
<details>
  <summary>Click me</summary>

  > FakeSymbol is an invention of LiteLDev HeaderGen, which was originally used to access member functions/variables with private/protected permissions. Since the access specifier is part of the function signature, it is not possible to simply change "private" to "public" in the header file.  
Now, it is also used to help developers hook special member functions in the class.  
As we all know, in C++, it is not possible to directly get the address of special member functions such as constructors/destructors/virtual functions. If you want to hook them, you must use string symbols, but string symbols are dirty. Another solution is to generate thunks and intercept symbol resolution, which is exactly what FakeSymbol does.

</details>

*TL;DR* FakeSymbol has many benefits, but it actually causes incorrect symbols to be generated, so it is necessary to restore them.

### Where to get the headers?
```
git clone https://github.com/LiteLDev/LeviLamina.git
git checkout -b header
```

### Thunks
 - `virtual function thunk` - *Removed.*
 - `constructor thunk` - *Removed.*
 - `vftables` - *Removed.*
   - **TODO** dumpsym is currently unable to emit a symbol of vtable pointer.
 - `destructor thunk` - *Removed.*
   - **TODO** dumpsym is currently unable to emit a symbol of destructor.
 - `static variables thunk` - *Restored.*
   - **TODO** Symbols are now generated correctly, but some symbols cannot be queried from bedrock_runtime_data.

### Usage
> [!NOTE]
> The modification will be done directly on the original file.
```
python src/main.py path/to/header/
```