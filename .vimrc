let g:syntastic_cpp_compiler='g++'
let g:syntastic_cpp_compiler_options='-std=c++14 -stdlib=libc++'
""let g:ycm_global_ycm_extra_conf='~/.vim/ycm_extra_conf.py'
let g:ycm_server_python3_interpreter = '/usr/bin/python'
let g:ycm_global_ycm_extra_conf='~/.vim/bundle/YouCompleteMe/third_party/ycmd/cpp/ycm/.ycm_extra_conf.py'
let g:ycm_collect_identifiers_from_comments_and_strings = 1
let g:ycm_seed_identifiers_with_syntax = 1
let g:ycm_add_preview_to_completeopt = 1
let g:ycm_autoclose_preview_window_after_completion = 1
let g:ycm_autoclose_preview_window_after_insertion = 1

set number
syntax on
"" vundle
set nocompatible
filetype off
set rtp+=~/.vim/bundle/Vundle.vim
call vundle#begin()
" alternatively, pass a path where Vundle should install plugins
"call vundle#begin('~/some/path/here')

" let Vundle manage Vundle, required
Plugin 'VundleVim/Vundle.vim'

" The following are examples of different formats supported.
" Keep Plugin commands between vundle#begin/end.
" plugin on GitHub repo
Plugin 'tpope/vim-fugitive'
Plugin 'wincent/command-t.git'
Plugin 'rstacruz/sparkup', {'rtp': 'vim/'}

Plugin 'seebi/dircolors-solarized'
Plugin 'altercation/solarized'
Plugin 'Valloric/YouCompleteMe'
Plugin 'Valloric/ListToggle'
Plugin 'scrooloose/syntastic'

call vundle#end()
filetype plugin indent on
"" vundle

syntax enable
""if has('gui_running')
""    set background=light
""else
""    set background=dark
""endif
""colorscheme solarized
set guifont=Menlo:h14
set ts=2
set shiftwidth=2
set softtabstop=2
set expandtab
set autoindent

""set fdm=indent
""set fdm=syntax

:imap <F1> <ESC>
:map <F1> <ESC>
inoremap ( ()<ESC>i
""inoremap ) ()
inoremap [ []<ESC>i
inoremap { {}<ESC>i
""inoremap { {}<ESC>i<RETURN><ESC>O
""inoremap #in #include <><ESC>i
""inoremap if if () {}<ESC>i<RETURN><ESC>k0f)i
""inoremap for for () {}<ESC>i<RETURN><ESC>k0f)i
""inoremap whi while () {}<ESC>i<RETURN><ESC>k0f)i
inoremap " ""<ESC>i
