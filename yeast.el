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

(defvar yeast-modes
  '((python-mode . python))
  "Alist of major modes and yeast language identifiers")

(defun yeast-parse ()
  "Parse the buffer from scratch."
  (let ((inhibit-redisplay t)
        (multibyte enable-multibyte-characters))
    (set-buffer-multibyte nil)
    (unwind-protect
        (yeast--parse yeast--instance)
      (set-buffer-multibyte multibyte))))

;;;###autoload
(define-minor-mode yeast-mode
  "Structural editing support."
  nil nil nil
  (if yeast-mode
      (let ((lang (catch 'found
                    (dolist (elt yeast-modes)
                      (when (derived-mode-p (car elt)) (throw 'found (cdr elt)))))))
        (if lang
            (progn
              (setq-local yeast--instance (yeast--make-instance lang))
              (yeast-parse))
          (user-error "Yeast does not support this major mode")
          (setq-local yeast-mode nil)))
    (setq-local yeast--instance nil)))

(provide 'yeast)

;;; yeast.el ends here
