pub struct Reporter {
    fail_fast: bool,
}

impl Reporter {
    pub const fn new(fail_fast: bool) -> Self {
        Self { fail_fast }
    }
}
