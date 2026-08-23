# Graphics API Demo

This example exercises the C-callable graphics API with named mode constants,
bitmap primitives, sprites, tile patterns, and wrong-mode error handling.

The executable changes the display mode, so its header does not promise to
leave the ForceCommand display state untouched on return.
