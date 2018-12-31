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

(require 'cl-lib)


;;; Loading logic

;; TODO: Improve after fix for Emacs bug #33231 is pulled

(defvar libyeast--root
  (file-name-directory (or load-file-name buffer-file-name)))

(defvar libyeast--build-dir
  (expand-file-name "build" libyeast--root))

(defvar libyeast--module-file
  (expand-file-name (concat "libyeast" module-file-suffix) libyeast--build-dir))

(unless (featurep 'libyeast)
  (load-file libyeast--module-file))


;;; Utility macros

(defmacro yeast-with-unibyte (&rest body)
  (declare (indent 0))
  `(let ((inhibit-redisplay t)
         (multibyte enable-multibyte-characters))
     (set-buffer-multibyte nil)
     (unwind-protect
         (progn ,@body)
       (set-buffer-multibyte multibyte))))


;;; Tracking changes

(defvar-local yeast--before-change-data nil)

(defun yeast--before-change (beg end)
  (setq-local yeast--before-change-data
              (cons beg (buffer-substring-no-properties beg end))))

(defun yeast--after-change (beg end len)
  (let* ((pre-beg (car yeast--before-change-data))
         (pre-str (cdr yeast--before-change-data))
         (i1 (- beg pre-beg))
         (i2 (+ i1 len))
         (nbytes (string-bytes (substring pre-str i1 i2))))
    (setq yeast--before-change-data
          (cons pre-beg
                (concat (substring pre-str 0 i1)
                        (buffer-substring-no-properties beg end)
                        (substring pre-str i2))))
    (yeast-with-unibyte
      (yeast--edit yeast--instance
                   (1- (position-bytes beg))
                   (1- (position-bytes end))
                   nbytes))))


;;; Yeast minor mode

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
  (when yeast--instance
    (yeast-with-unibyte
      (yeast--parse yeast--instance))))

(defun yeast-root-node ()
  "Get the current root node."
  (yeast--tree-root (yeast--instance-tree yeast--instance)))

(defvar yeast-mode-map
  (make-sparse-keymap)
  "Keymap for yeast-mode.")

;;;###autoload
(define-minor-mode yeast-mode
  "Structural editing support."
  nil nil yeast-mode-map
  (if yeast-mode
      (if-let ((lang (yeast-detect-language)))
          (progn
            (setq-local yeast--instance (yeast--make-instance lang))
            (yeast-parse)
            (add-hook 'before-change-functions 'yeast--before-change nil t)
            (add-hook 'after-change-functions 'yeast--after-change nil t))
        (user-error "Yeast does not support this major mode")
        (setq-local yeast-mode nil))
    (remove-hook 'before-change-functions 'yeast--before-change t)
    (remove-hook 'after-change-functions 'yeast--after-change t)
    (setq-local yeast--instance nil)))


;;; Convenience functionality

(defun yeast--node-at-point (point mark)
  (let* ((min-char (min point mark))
         (max-char (max (1- (max point mark)) min-char))
         (min-byte (position-bytes min-char))
         (max-byte (position-bytes max-char))
         (node (yeast-root-node))
         prev-node)
    ;; Descend until we find the youngest node containing the range
    (while (not (null node))
      (let* ((min-node (yeast--node-child-for-byte node min-byte))
             (max-node (yeast--node-child-for-byte node max-byte))
             (byte-range (and min-node (yeast--node-byte-range min-node))))
        (setq prev-node node
              node (and (yeast-node-eq min-node max-node)
                        (>= min-byte (car byte-range))
                        (<= max-byte (cdr byte-range))
                        min-node))))
    ;; Ascend until we find the oldest node that is not wider than the youngest
    (let ((parent (yeast--parent prev-node))
          (range (yeast--node-byte-range prev-node)))
      (while (and parent (equal range (yeast--node-byte-range parent)))
        (setq prev-node parent
              parent (yeast--parent prev-node))))
    prev-node))

(defun yeast--select-node (node)
  (when node
    (let ((byte-range (yeast--node-byte-range node)))
      (goto-char (byte-to-position (car byte-range)))
      (set-mark (1+ (byte-to-position (cdr byte-range)))))))

(defun yeast-node-children (node &optional anon)
  "Get the children of NODE.
If ANON is nil, get only the named children."
  (when node
    (let ((nchildren (yeast--node-child-count node anon)))
      (cl-loop for i below nchildren collect (yeast--node-child node i anon)))))

(defun yeast-ast-sexp (&optional node anon)
  "Convert NODE to an s-expression.
If NODE is nil, use the current root node.
If ANON is nil, only use the named nodes."
  (let* ((node (or node (yeast-root-node)))
         (children (yeast-node-children node anon)))
    `(,(yeast--node-type node)
      ,@(cl-loop for node in children collect (yeast-ast-sexp node anon)))))


;;; Traversal by selection

(defun yeast-select-at-point (point mark)
  (interactive (list (point) (if (use-region-p) (mark) (point))))
  (yeast--select-node (yeast--node-at-point point mark)))

(defun yeast-select-next-at-point (point mark)
  (interactive (list (point) (if (use-region-p) (mark) (point))))
  (let ((cur-node (yeast--node-at-point point mark)))
    (yeast--select-node (yeast--next-sibling cur-node))))

(defun yeast-select-prev-at-point (point mark)
  (interactive (list (point) (if (use-region-p) (mark) (point))))
  (let ((cur-node (yeast--node-at-point point mark)))
    (yeast--select-node (yeast--prev-sibling cur-node))))

(defun yeast-select-parent-at-point (point mark)
  (interactive (list (point) (if (use-region-p) (mark) (point))))
  (let* ((node (yeast--node-at-point point mark))
         (range (yeast--node-byte-range node)))
    ;; Get the closest ancestor with a wider byte range
    (while (and node (equal range (yeast--node-byte-range node)))
      (setq node (yeast--parent node)))
    (yeast--select-node node)))

(defun yeast-select-first-child-at-point (point mark)
  (interactive (list (point) (if (use-region-p) (mark) (point))))
  (let* ((node (yeast--node-at-point point mark))
         (range (yeast--node-byte-range node)))
    ;; Get the primary descendant with a narrower byte range
    (while (and (equal range (yeast--node-byte-range node))
                (< 0 (yeast--node-child-count node)))
      (setq node (yeast--node-child node 0)))
    (yeast--select-node node)))


;;; Tree display

(defun yeast--tree-widget (node &optional anon)
  "Convert NODE to a tree widget.
If ANON is nil, only use named nodes."
  (require 'wid-edit)
  (require 'tree-widget)
  (widget-convert 'tree-widget
                  :tag (format "%s (%d - %d) (%d - %d)"
                               (yeast--node-type node)
                               (car (yeast--node-byte-range node))
                               (cdr (yeast--node-byte-range node))
                               (byte-to-position (car (yeast--node-byte-range node)))
                               (byte-to-position (cdr (yeast--node-byte-range node))))
                  :open t
                  :args (cl-loop for node in (yeast-node-children node anon)
                                 collect (yeast--tree-widget node anon))))

(defun yeast-show-ast (&optional anon)
  "Show the AST in a separate buffer.
If ANON is nil, only use the named nodes."
  (interactive "P")
  (let ((buffer (generate-new-buffer "*yeast-tree*"))
        (widget (yeast--tree-widget (yeast-root-node) anon)))
    (with-current-buffer buffer
      (setq-local buffer-read-only t)
      (let ((inhibit-read-only t))
        (erase-buffer)
        (widget-create widget)
        (goto-char (point-min))))
    (switch-to-buffer-other-window buffer)))

(provide 'yeast)

;;; yeast.el ends here
