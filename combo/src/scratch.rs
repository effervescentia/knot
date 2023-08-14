use std::{
    cell::{OnceCell, Ref, RefCell},
    rc::{Rc, Weak},
};

pub struct State<T> {
    position: RefCell<usize>,
    buffer: Vec<T>,
    iterator: OnceCell<Box<dyn Iterator<Item = T>>>,
}

impl<T> State<T> {
    pub fn new(iterator: Box<dyn Iterator<Item = T>>) -> Self {
        let state = State {
            position: RefCell::new(0 as usize),
            buffer: vec![],
            iterator: OnceCell::new(),
        };

        state.iterator.set(iterator).ok();

        state
    }

    pub fn get_position(&self) -> Ref<'_, usize> {
        self.position.borrow()
    }

    fn with_iterator(&mut self, f: impl FnOnce(&mut Self, &mut Box<dyn Iterator<Item = T>>)) {
        self.iterator.take().map(|mut iterator| {
            f(self, &mut iterator);
            self.iterator.set(iterator).ok();
        });
    }

    pub fn advance_to(&mut self, target: usize) {
        let mut position = *self.get_position();

        while position < target {
            self.with_iterator(|state, iterator| {
                iterator.next().map(|value| {
                    position += 1;
                    state.buffer.push(value);
                });
            })
        }

        self.position.replace(position);
    }

    pub fn get(&self, target: usize) -> Option<&T> {
        let position = *self.get_position();

        if target > position {
            None
        } else {
            self.buffer.get(target - position)
        }
    }
}

pub struct Stream<T> {
    state: State<T>,
    cursors: RefCell<Vec<Weak<RefCell<Cursor>>>>,
}

impl<T> Stream<T> {
    pub fn new(iterator: Box<dyn Iterator<Item = T>>) -> Self {
        Stream {
            state: State::new(iterator),
            cursors: RefCell::new(vec![]),
        }
    }

    pub fn add_cursor(self, cursor: Rc<RefCell<Cursor>>) {
        self.cursors.borrow_mut().push(Rc::downgrade(&cursor));
    }

    pub fn next(&mut self, cursor: Rc<RefCell<Cursor>>) -> Option<&T> {
        let mut cursor = cursor.borrow_mut();
        let next_position = cursor.position + 1;

        self.state.advance_to(next_position);

        self.state.get(next_position).map(|next| {
            cursor.position = next_position;
            next
        })
    }
}

pub struct Cursor {
    position: usize,
}

impl Cursor {
    pub fn new<T>(stream: &Stream<T>) -> Rc<RefCell<Self>> {
        let cursor = Rc::new(RefCell::new(Cursor { position: 0 }));

        stream.cursors.borrow_mut().push(Rc::downgrade(&cursor));

        cursor
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        let vec = vec![1, 2, 3];

        let mut stream = Stream::new(Box::new(vec.into_iter()));
        let cursor = Cursor::new(&stream);

        assert_eq!(stream.next(cursor), Some(&1));
    }
}
