;;; yeast.el --- Structural editing. -*- lexical-binding: t; -*-

;; Copyright (C) 2018 TheBB
;;
;; Author: Eivind Fonn <evfonn@gmail.com>
;; URL: https://github.com/TheBB/yeast
;; Version: 0.0.1
;; Keywords: git vc
;; Package-Requires: ((emacs "25.1"))

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

;;; Commentary:

;; This package provides bindings to tree-sitter.

;;; Code:

(defvar libyeast--root
  (file-name-directory (or load-file-name buffer-file-name)))

(defvar libyeast--build-dir
  (expand-file-name "build" libyeast--root))

(defvar libyeast--module-file
  (expand-file-name (concat "libyeast" module-file-suffix) libyeast--build-dir))

(unless (featurep 'libyeast)
  (load-file libyeast--module-file))

(defvar-local yeast--instance nil
  "The yeast instance of the current buffer.")

(defun yeast-detect-language ()
  "Detect a supported language in the current buffer."
  (cond
   ((and (derived-mode-p 'sh-mode)
         (string= sh-shell "bash"))
    'bash)
   ((derived-mode-p 'c-mode) 'c)
   ((derived-mode-p 'c++-mode) 'cpp)
   ((derived-mode-p 'css-mode) 'css)
   ((derived-mode-p 'go-mode) 'go)
   ((or (derived-mode-p 'html-mode)
        (and (derived-mode-p 'web-mode)
             (string= web-mode-engine "none")))
    'html)
   ((derived-mode-p 'json-mode) 'json)
   ((derived-mode-p 'javascript-mode 'js-mode 'js2-mode 'js3-mode) 'javascript)
   ((derived-mode-p 'tuareg-mode) 'ocaml)
   ((derived-mode-p 'php-mode) 'php)
   ((derived-mode-p 'python-mode) 'python)
   ((derived-mode-p 'ruby-mode 'enh-ruby-mode) 'ruby)
   ((derived-mode-p 'rust-mode) 'rust)
   ((derived-mode-p 'typescript-mode) 'typescript)))

(defun yeast-parse ()
  "Parse the buffer from scratch."
  (let ((inhibit-redisplay t)
        (multibyte enable-multibyte-characters))
    (set-buffer-multibyte nil)
    (unwind-protect
        (yeast--parse yeast--instance)
      (set-buffer-multibyte multibyte))))

(defun yeast-root-node ()
  "Get the current root node."
  (yeast--tree-root (yeast--instance-tree yeast--instance)))

(defun yeast-node-children (node &optional anon)
  "Get the children of NODE.
If ANON is nil, get only the named children."
  (when node
    (let ((nchildren (yeast--node-child-count node)))
      (cl-loop for i below nchildren collect (yeast--node-child node i anon)))))

(defun yeast-ast-sexp (&optional node anon)
  "Convert NODE to an s-expression.
If NODE is nil, use the current root node.
If ANON is nil, only use the named nodes."
  (let* ((node (or node (yeast-root-node)))
         (children (yeast-node-children node anon)))
    `(,(yeast--node-type node)
      ,@(mapcar (lambda (node) (yeast-ast-sexp node anon)) children))))

(defun yeast--tree-widget (node &optional anon)
  "Convert NODE to a tree widget.
If ANON is nil, only use named nodes."
  (require 'wid-edit)
  (require 'tree-widget)
  (widget-convert 'tree-widget
                  :tag (symbol-name (yeast--node-type node))
                  :open t
                  :args (mapcar (lambda (node) (yeast--tree-widget node anon))
                                (yeast-node-children node))))

(defun yeast-show-ast (&optional anon)
  "Show the AST in a separate buffer.
If ANON is nil, only use the named nodes."
  (interactive "P")
  (let ((buffer (generate-new-buffer "*yeast-tree*"))
        (widget (yeast--tree-widget (yeast-root-node))))
    (with-current-buffer buffer
      (setq-local buffer-read-only t)
      (let ((inhibit-read-only t))
        (erase-buffer)
        (widget-create widget)
        (goto-char (point-min))))
    (switch-to-buffer-other-window buffer)))

;;;###autoload
(define-minor-mode yeast-mode
  "Structural editing support."
  nil nil nil
  (if yeast-mode
      (if-let ((lang (yeast-detect-language)))
          (progn
            (setq-local yeast--instance (yeast--make-instance lang))
            (yeast-parse))
        (user-error "Yeast does not support this major mode")
        (setq-local yeast-mode nil))
    (setq-local yeast--instance nil)))

(provide 'yeast)

;;; yeast.el ends here
