

# MoonNet log





## Changelog

### v1.0.2

- Fix the issue where the max_thr_num variable in the Threadpool is not set

### v1.0.1

- Updated the `base_event` API to unify operations across various event types.
- Refactored the `server` event handling approach to standardize operations for different event types. **Deprecated non-specialized event handling APIs (e.g., `add_bev(bfevent* bev)`) and adopted a general API such as `addev(base_event* ev)`**.

- Updated copyright.

​	

### v1.0.0

- **Initial Release**: MoonNet network library first released, supporting TCP and UDP event handling based on a master-worker Reactor multithreaded model.
- **Core Features**:
  - Implementation of event loops (`eventloop`) and event handling (`event`).
  - Multithreading support through `loopthread` and `looptpool` managing multiple event loops.
  - TCP connection management with buffered event handling based on `bfevent`.
  - UDP support via `udpevent` for data reception and transmission.
  - Timer event handling implemented through `timerevent` for scheduled tasks.
  - Signal event handling using `signalevent` to capture and process system signals.
  - Static/dynamic load balancing
- **Utility Classes**:
  - Buffer management (`buffer`) for efficient data read/write operations.
  - Platform-independent socket operations encapsulation (`wrap`).
- **Server Framework**:
  - `server` class integrating all components to simplify server setup process.
