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
