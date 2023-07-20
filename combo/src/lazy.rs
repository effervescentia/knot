use std::cell;

pub struct Lazy<'a, T> {
    cell: cell::OnceCell<T>,
    factory: &'a mut dyn FnMut() -> T,
}

impl<'a, T> Lazy<'a, T> {
    pub fn new(factory: &'a mut impl FnMut() -> T) -> Lazy<'a, T> {
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
        let mut factory = || 123;
        let mut lazy = Lazy::new(&mut factory);

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
        let mut lazy = Lazy::new(&mut factory);

        assert_eq!(*lazy.get(), 1);
        assert_eq!(*lazy.get(), 1)
    }
}

// pub struct Lazy2<'a, T> {
//     cell: cell::OnceCell<T>,
//     resolve: &'a dyn FnOnce() -> T,
// }

// impl<'a, T> Lazy2<'a, T> {
//     pub fn new(factory: &'a impl FnOnce() -> T) -> Lazy2<'a, T> {
//         let cell = cell::OnceCell::new();
//         // let resolve = || cell.get_or_init(factory);

//         Lazy2 {
//             cell,
//             resolve: &(|| factory()),
//         }
//     }

//     // pub fn get(&mut self) -> &T {
//     //     self.cell.get_or_init(|| (self.factory)())
//     // }
// }

// #[cfg(test)]
// mod tests2 {
//     use super::*;

//     #[test]
//     fn get_from_factory() {
//         let mut factory = || 123;
//         let mut lazy = Lazy2::new(&mut factory);

//         assert_eq!(*lazy.get(), 123)
//     }

//     #[test]
//     fn only_call_factory_once() {
//         let mut counter = 0;
//         let mut increment_and_return = || {
//             counter = counter + 1;
//             counter
//         };
//         let mut factory = || increment_and_return();
//         let mut lazy = Lazy2::new(&mut factory);

//         assert_eq!(*lazy.get(), 1);
//         assert_eq!(*lazy.get(), 1)
//     }
// }
