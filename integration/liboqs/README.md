# liboqs integration

Metadata and glue code that let `liboqs`' `scripts/copy_from_upstream` pull SDitH3
into `src/sig/sdith/`. Nothing here is used by a standalone SDitH build.

The different files involves in the integration are:
- `META_Common.yml`: the shared object libraries liboqs builds, and their sources
- `schemes/META_{scheme}.yml`: one file per scheme variant specifying sizes, submitters, compile options
- `api.c`, `api.h`, `check_params.h`: the liboqs wrapper around `sdith_sign` / `sdith_verify`
- `aes_glue.h`, `fips202_glue.h`, `fips202x4_glue.h`: shim SDitH's AES and Keccak at liboqs' implementations where appropriate
- `sdith_namespace.h`: generated symbol prefixes (see below)
- `gen_namespace.sh`: regenerates `sdith_namespace.h`

## KAT checksums

`liboqs` runs the NIST KATs to verify correctness of the integration.
Instead of comparing the results strictly, the SHA256 checksum of the output is compared instead.
This is computed as the hash of the standard NIST output format. For example (truncated):
```
count = 0
seed = 061550234D158C5EC95595FE04EF7A25767F2E24CC2BC479D09D86DC9ABCFDE7056A8C266F9EF97ED08541DBD2E1FFA1
mlen = 33
msg = D81C4D8D734FCBFBEADE3D3F8A039FAA2A2C9957E835AD55B22E75BF57BB556AC8
pk = 7C9935A0B...
sk = 7C9935A...
smlen = 4947
sm = D81C...
```
The above output (with `count=0`) is the hash input for the `nistkat-sha256` checksum included in each scheme's `META_*.yml`.
The `all` hash must be added to liboqs' `tests/KATs/sig/kats.json` by hand, and consists of the hash of all of the above outputs for `count`s 0 through 99, with each followed by a newline.

As of writing, sdith comes with their KAT outputs at `kat_r3/<SCHEME>/PQCsignKAT_*.rsp` (eg. `kat_r3/SDITH_CAT1_FAST/PQCsignKAT_147.rsp`)

Getting the checksum for a single test (`nistkat-sha256` in the `.yml`) requires hashing the first output block (ignoring the comment at the top of each `.rsp` file):
```sh
tail -n +3 kat_r3/SDITH_CAT1_FAST/PQCsignKAT_147.rsp | head -n 8  | sha256sum
```
and getting the "all" checksum involves hashing the whole file (ignoring the comment and extra trailing newline)
```sh
tail -n +3 kat_r3/SDITH_CAT1_FAST/PQCsignKAT_147.rsp | head -n -1 | sha256sum
```

## Namespacing

`liboqs` compiles the common code into a set of global symbols
that can be shared among each scheme. As a result, the global symbols within
`sdith` need to be namespaced to meet liboqs' convention and avoid conflicts.

Instead of updating hundereds of symbols within the `sdith` source, a generated
`sdith_namespace.h` header uses `# define` to prefix global defined symbols.

Regenerate it after adding, renaming, or removing any symbol in the shared
sources:

```sh
./integration/liboqs/gen_namespace.sh /path/to/liboqs/build
# or, if $LIBOQS_BUILD_DIR is defined:
# ./integration/liboqs/gen_namespace.sh $LIBOQS_BUILD_DIR
```

The directory is searched for the object files of the `sdith_common` targets
under `src/sig/sdith/CMakeFiles/sdith_sdith_common*.dir/`.

`sdith_common_avx2` is only compiled on x86_64, so use an x86_64 build tree or
the header will be missing the avx2 symbols. 

Re-running over a build that already applied the header will not update it
(avoids adding the prefix twice).

## Reference vs avx2

The two copies carry different `APPLY_NAMESPACE` prefixes: `oqs_sig_private_sdith_ref_` and
`oqs_sig_private_sdith_avx2_` depending on the compiled implimentation variant.
Each implimentation variant's `api.c` is compiled with the prefix of the copy it links against.
`gen_namespace.sh` strips either prefix, so the single generated header serves both.
