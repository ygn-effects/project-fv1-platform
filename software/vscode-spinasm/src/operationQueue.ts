/**
 * Runs async operations one at a time, in the order they were queued.
 *
 * Compiles and uploads share output files and the programmer's serial port, so
 * two of them running at once (e.g. compile-on-save during an upload) could
 * rewrite a .hex that is being read or open the port twice. A failed operation
 * doesn't block the ones queued after it.
 *
 * Pure and dependency-free so the ordering can be unit-tested.
 */
export class OperationQueue {
  private tail: Promise<void> = Promise.resolve();
  private pending = 0;

  /** True while an operation is running or waiting to run. */
  public get isBusy(): boolean {
    return this.pending > 0;
  }

  /** Queues `operation` and resolves or rejects with its result once it has run. */
  public run<T>(operation: () => Promise<T>): Promise<T> {
    this.pending++;

    const result = this.tail.then(operation);
    const settled = () => {
      this.pending--;
    };
    this.tail = result.then(settled, settled);

    return result;
  }
}
