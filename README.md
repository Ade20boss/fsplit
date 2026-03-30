# fsplit — File Splitter & Assembler

A lightweight, dependency-free command-line toolkit written in C for splitting large files into numbered parts and reassembling them back into the original file. No external libraries. No overhead. Just raw file I/O.

---

## Why?

- Transferring large files over services with size limits (email, Discord, etc.)
- Storing large files across multiple smaller storage media
- Chunking files for manual checkpointing or partial transfers
- Learning how binary file I/O works at a low level

---

## Tools

| Binary | Description |
|--------|-------------|
| `fsplit` | Splits a file into N equal parts (last part absorbs the remainder) |
| `fassemble` | Reassembles split parts back into the original file |

---

## Building

Requires GCC (or any C99-compatible compiler) and a Unix-like environment.

```bash
# Compile the splitter
gcc -o fsplit splitter.c

# Compile the assembler
gcc -o fassemble assembler.c
```

Or with warnings enabled (recommended):

```bash
gcc -Wall -Wextra -o fsplit splitter.c
gcc -Wall -Wextra -o fassemble assembler.c
```

---

## Usage

### Splitting a file

```bash
./fsplit <filename> <number_of_parts>
```

**Example:**

```bash
./fsplit archive.zip 4
```

**Output:**

```
File_size: 104857600 bytes
Part_size: 26214400 bytes
Remainder_size: 0 bytes
archive.zip_1
archive.zip_2
archive.zip_3
archive.zip_4
```

This produces `archive.zip_1`, `archive.zip_2`, `archive.zip_3`, `archive.zip_4` in the current directory. The original file is left untouched.

---

### Assembling parts

```bash
./fassemble <original_filename> <number_of_parts>
```

**Example:**

```bash
./fassemble archive.zip 4
```

**Output:**

```
Starting reassembly of 4 parts...
Merged archive.zip_1 successfully.
Merged archive.zip_2 successfully.
Merged archive.zip_3 successfully.
Merged archive.zip_4 successfully.
Reassembly complete. Cleaning up part files...
Done. Final file: restored_archive.zip
```

This produces `restored_archive.zip` and deletes the part files. The assembler will refuse to run if `restored_archive.zip` already exists.

---

## How It Works

### Splitter

1. Opens the source file in binary read mode (`rb`)
2. Determines total file size via `fseek` / `ftell`
3. Computes `part_size = filesize / N` and `remainder = filesize % N`
4. Iterates N times, writing `part_size` bytes to each numbered part file
5. The final part receives `part_size + remainder` to account for uneven division
6. All I/O is done in `4096`-byte chunks — the source file is never fully loaded into memory

### Assembler

1. Validates that the output file (`restored_<filename>`) does not already exist
2. Opens each numbered part in order (`filename_1`, `filename_2`, ...)
3. Reads each part in `4096`-byte chunks, writing sequentially to the output file
4. Only after all parts are successfully merged does it delete the part files
5. Cleanup is intentionally deferred — if reassembly fails midway, part files are preserved

---

## Design Decisions

**Chunked I/O (`READ_SIZE 4096`)**
Files are never fully loaded into memory. A single 4 KB buffer handles arbitrarily large files, making this suitable for multi-gigabyte inputs.

**Remainder handling**
Integer division truncates, so `filesize / N` may lose bytes. The last part absorbs the remainder, ensuring byte-perfect reconstruction with no data loss.

**Overwrite protection**
The assembler checks for an existing `restored_<filename>` before creating anything. This prevents silent data loss on accidental re-runs.

**Deferred cleanup**
Part files are only deleted after the full output file is written and closed. A failed mid-reassembly run leaves all parts intact so the operation can be retried.

**Binary mode (`rb` / `wb`)**
All files are opened in binary mode, making the tools format-agnostic. Text, images, archives, executables — anything works.

---

## Limitations

- Filenames cannot exceed 245 characters (to accommodate the `_N` suffix within a 256-byte buffer)
- Part files must be in the same directory as the binary when assembling
- No integrity verification (checksums) — a corrupted part will produce a corrupted output silently
- No encryption or compression

---

## Potential Improvements

- [ ] Add MD5/SHA256 checksum verification per part
- [ ] Add a manifest file (`.fsplit`) generated at split time to simplify reassembly
- [ ] Support custom output directory via flag
- [ ] Progress bar for large files
- [ ] Parallel reassembly for faster merging on SSDs

---

## License

MIT — do whatever you want with it.
