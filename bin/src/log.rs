use crate::config::{Config, ConfigList};
use command::Phase;

pub fn configuration(configs: Vec<(&'static str, Config)>) {
    eprintln!("{}{}", Phase::Configuration, ConfigList::new(configs));
}
