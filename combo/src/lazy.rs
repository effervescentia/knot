use std::cell;

pub struct Lazy<T, F: FnMut() -> T> {
    cell: cell::OnceCell<T>,
    factory: F,
}

impl<T, F: FnMut() -> T> Lazy<T, F> {
    pub fn new(factory: F) -> Lazy<T, F> {
        Lazy {
            cell: cell::OnceCell::new(),
            factory,
        }
    }

    pub fn get(&mut self) -> &T {
        self.cell.get_or_init(|| (self.factory)())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn get_from_factory() {
        let mut lazy = Lazy::new(|| 123);

        assert_eq!(*lazy.get(), 123)
    }

    #[test]
    fn only_call_factory_once() {
        let mut counter = 0;
        let mut increment_and_return = || {
            counter = counter + 1;
            counter
        };
        let mut lazy = Lazy::new(|| increment_and_return());

        assert_eq!(*lazy.get(), 1);
        assert_eq!(*lazy.get(), 1)
    }
}

pub struct Lazy3<'a, T> {
    cell: cell::OnceCell<T>,
    factory: &'a mut dyn FnMut() -> T,
}

impl<'a, T> Lazy3<'a, T> {
    pub fn new(factory: &'a mut impl FnMut() -> T) -> Lazy3<'a, T> {
        Lazy3 {
            cell: cell::OnceCell::new(),
            factory,
        }
    }

    pub fn get(&mut self) -> &T {
        // match self.cell.get() {
        //     Some(value) => value,
        //     None => {
        //         let value_ref = (self.factory)();

        //         self.cell.set(value_ref);

        //         &value_ref
        //     }
        // }
        self.cell.get_or_init(|| (self.factory)())
    }
}

#[cfg(test)]
mod tests3 {
    use super::*;

    #[test]
    fn get_from_factory() {
        let mut factory = || 123;
        let mut lazy = Lazy3::new(&mut factory);

        assert_eq!(*lazy.get(), 123)
    }

    #[test]
    fn only_call_factory_once() {
        let mut counter = 0;
        let mut increment_and_return = || {
            counter = counter + 1;
            counter
        };
        let mut factory = || increment_and_return();
        let mut lazy = Lazy3::new(&mut factory);

        assert_eq!(*lazy.get(), 1);
        assert_eq!(*lazy.get(), 1)
    }
}
