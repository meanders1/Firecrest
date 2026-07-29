# TODO

## Text

- [ ] Text: Make text selection with mouse work
- [ ] TextInput: Make hotkeys work (Ctrl+C, Ctrl+V, Ctrl+X, Ctrl+A, etc.)
- [ ] Support Unicode
- [ ] Emojis
- [x] Signed distance fields for text (FT_RENDER_MODE_SDF)?

## New features

- [ ] Make the Scene3D take an arbitrary Camera
- [ ] Make the display tree convertable to a string, so layouts can be loaded from strings.
- [ ] Sliders
- [x] Custom shaders
- [ ] Transitions/Animations
- [ ] Create Shape-objects that contains their own meshes, so ShapeRenderer2D does not have to rewrite its own for every shape drawn
- [ ] Scoped binding

## Modifications

- [ ] Flip the y-axis?
- [ ] Smoother scrolling (physics?)
- [ ] Split camera movement speed by axies
- [ ] Make it possible to run compute shaders without creating a window.
- [ ] Cache alignments
- [ ] Have caches for GL_DEPTH_TEST, GL_CULL_FACE etc. if there is a performance hit from calling them often.
- [x] Improve RAII