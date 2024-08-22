#[derive(Debug, Default, PartialEq)]
pub struct Incrementor(usize);

impl Incrementor {
    pub const fn from(initial: usize) -> Self {
        Self(initial)
    }

    pub fn increment(&mut self) -> usize {
        let next = self.0;
        self.0 += 1;
        next
    }
}
